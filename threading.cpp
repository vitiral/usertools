/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Threading library to make microcontroller threading simple, intuitive, and debuggable.
 *   See UserGuide_ui.html in the documentation.
 */
 
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "pt.h"

#include "errorhandling.h"

#include "threading.h"
//#include "strtools.h"
//#include "ui.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

// #####################################################
// ### Globals
TH_VariableArray TH__variables = {0, 0, 0};
TH_FunctionArray TH__functions = {0, 0, 0};
TH_ThreadArray TH__threads = {0, 0, 0};

const __FlashStringHelper **TH__thread_names;

uint8_t th_calling = 255;
uint8_t th_loop_index = 255;


// #####################################################
// ### Exported Functions
/*

uint8_t call_function(char *name, char *input){
  TH_function *fun = TH_get_function(name);
  if(fun == NULL) return false;
  th_calling = name;
  fun->fptr(input);
  th_calling = NULL;
  return true;
}

TH_variable *TH_get_variable(char *name){
  TH_variable *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  sdebug(F("gv: "));
  edebug(name);
  for(i = 0; i < TH__variables.index; i++){
    var = &TH__variables.array[i];
    debug(var->el.name);
    if(cmp_str_elptr(name, name_len, var)){
      sdebug("Match");
      edebug(var->el.name);
      return var;
    }
  }
  return NULL;
}

uint8_t thread_ui_loop(){
  uint64_t time;
  uint16_t timeus;
  uint8_t fout;
  uint16_t init_lc;
  thread *th;
  
  // ***** don't try this at home kids ********
  static struct PTsmall pt = {0};
  // So now "pt" will be (ptsmall *)(&pt)
  // ******************************************
  PT_BEGIN((ptsmall *)(&pt));
  while(true){
    PT_YIELD((ptsmall *)(&pt));
    for(th_loop_index = 0; th_loop_index < TH__threads.index; th_loop_index++){
      th = &TH__threads.array[th_loop_index];
      init_lc = th->pt.lc;
      if(init_lc == PT_INNACTIVE) continue;
      time = millis();
      timeus = micros();
      assert(th->fptr, th_loop_index);    // NOT NULL PTR
      fout = th->fptr(&(th->pt), EH_EMPTY_STR);
      timeus = (uint16_t)micros() - timeus;
      time = (uint64_t)millis() - time;

      if(time > 10){ // too long, disregard microseconds
        time = th->time + time * 100;
      }
      else{
        if(timeus > 1000){
          time = th->time + time * 100 + (timeus / 10);
        }
        else{
          // time too short for millis
          time = th->time + (timeus / 10);
        }
      }
      if(time > 65535) th->time = 65535;
      else th->time = time;

      if((fout >= PT_EXITED) || (init_lc == PT_KILL_VALUE)){
        if(fout < PT_EXITED){
          seterr(ERR_TYPE);
          log_err(F("NoDie"));
        }  
        set_thread_innactive(th);
        log_thread_exit(th);
      }
error:
      clrerr();
      PT_YIELD(  (ptsmall *)(&pt));
    }
  }
  PT_END((ptsmall *)(&pt));
}
*/


// *****************************************************************************
// **************  Small Threads


// *******************************
// **** Macro Helpers
void TH__set_variable_array(TH_variable *vray, uint16_t len);

void TH__set_function_array(TH_function *fray, uint16_t len);

void TH__set_thread_array(thread *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__threads.array = fray;
  TH__threads.len = len;
  for(uint8_t i = 0; i < len; i++){
    TH__threads.array[i].fptr = NULL;
    TH__threads.array[i].pt.lc = PT_INNACTIVE;
  }
}

//void TH__expose_variable(void *varptr, uint8_t varsize);

//void TH__expose_function(H_funptr fptr);

// #####################################################
// ### User Functions
TH_variable *get_variable(uint8_t el_num);

TH_function *get_function(uint8_t el_num);

void call_function(uint8_t el_num, char *input);


// #####################################################
// ### Package Access

// expose_variable = macro
// expose_function = macro

thread *expose_thread(TH_thfunptr fptr){
  debug(F("ExpSTh"));
  assert_return(TH__threads.index < TH__threads.len, NULL);
  assert_raise_return(TH__threads.array, ERR_VALUE, NULL); // assert not null

  TH__threads.array[TH__threads.index].fptr = fptr;
  TH__threads.array[TH__threads.index].pt.lc = PT_INNACTIVE;
  TH__threads.index++;
  sdebug(F("Added T:")); cdebug(TH__threads.index); edebug(F(" len:"));
  return &(TH__threads.array[TH__threads.index - 1]);
}

void set_thread_innactive(thread *th){
  th->pt.lc = PT_INNACTIVE;
}

uint8_t schedule_thread(thread *th, char *input){
  uint8_t out = false;
  
  sdebug(F("Calling:"));
  assert_raise(th->pt.lc == PT_INNACTIVE, ERR_VALUE, 0);
  PT_INIT(&(th->pt));
  assert(th->fptr);
  out = th->fptr(&(th->pt), EH_EMPTY_STR);
  if(out >= PT_EXITED){
    set_thread_innactive(th);
    out = true;
  }
  else{
    //log_thread_start(th);
    out = true;
  }
error:
  return out;
}

uint8_t schedule_thread(uint8_t el_num, char *input){
  thread *th = get_thread(el_num);
  assert_raise_return(th, ERR_VALUE, false);
  schedule_thread(th, input);
  return true;
}

thread *expose_run_thread(TH_thfunptr fun, char *input){
  thread *th = expose_thread(fun);
  iferr_log_return(NULL);
  schedule_thread(th, input);
  return th;
}

thread *expose_run_thread(TH_thfunptr fun){
  return expose_run_thread(fun, EH_EMPTY_STR);
}

thread *get_thread(uint8_t el_num){
  uint8_t i;
  assert_raise_return(el_num < TH__threads.len, ERR_INDEX, NULL);
  return &TH__threads.array[el_num];
}

void kill_thread(thread *th){
  th->pt.lc = PT_KILL_VALUE;
}

void kill_thread(uint8_t el_num){
  thread *th = get_thread(el_num);
  assert_raise_return(th, ERR_VALUE);
  kill_thread(th);
}

uint8_t thread_loop(){
  uint16_t init_lc;
  uint8_t fout;
  thread *th;
  
  // ***** don't try this at home kids ********
  static struct PTsmall pt = {0};
  // So now "pt" will be (ptsmall *)(&pt)
  // ******************************************
  PT_BEGIN((ptsmall *)(&pt));
  while(true){
    PT_YIELD((ptsmall *)(&pt));
    for(th_loop_index = 0; th_loop_index < TH__threads.index; th_loop_index++){
      th = &TH__threads.array[th_loop_index];
      init_lc = th->pt.lc;
      if(init_lc == PT_INNACTIVE) continue;
      assert(th->fptr, th_loop_index);    // NOT NULL PTR
      
      fout = th->fptr(&(th->pt), EH_EMPTY_STR);

      if((fout >= PT_EXITED) || (init_lc == PT_KILL_VALUE)){
        if(fout < PT_EXITED){
          seterr(ERR_TYPE);
          log_err(F("NoDie"));
        }
        set_thread_innactive(th);
        //log_thread_exit(th);
      }
error:
      clrerr();
      PT_YIELD((ptsmall *)(&pt));
    }
  }
  PT_END((ptsmall *)(&pt));
}




