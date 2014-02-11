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
#include "strtools.h"
//#include "ui.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

// #####################################################
// ### Globals
TH_Variables TH__variables = {0, 0, 0};
TH_Functions TH__functions = {0, 0, 0};
TH_ThreadArray TH__threads = {0, 0, 0};

char *th_calling = NULL;
uint8_t th_loop_index = 255;

// Small Threads
TH_sThreadArray TH__sthreads = {0, 0, 0};

// #####################################################
// ### Macro Helpers

// Variable Array
void TH__set_variable_array(TH_variable *vray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__variables.array = vray;
  TH__variables.len = len;
  TH__variables.index = 0;
}

void TH__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize){
  assert_return(TH__variables.index <= TH__variables.len);
  assert_return(flash_len(name) <= TH_MAX_NAME_LEN);
  assert_raise_return(TH__variables.array, ERR_VALUE); // assert not null
  uint8_t len = flash_len(name);
  assert_raise_return(len > 0, ERR_VALUE);

  TH__variables.array[TH__variables.index].el.name = name;
  TH__variables.array[TH__variables.index].el.name_len = len;
  TH__variables.array[TH__variables.index].vptr = varptr;
  TH__variables.array[TH__variables.index].size = varsize;
  sdebug(F("Added V:")); cdebug(name); cdebug(F(" len:"));
  edebug(TH__variables.array[TH__variables.index].el.name_len);
  TH__variables.index++;
}

// Function Array
void TH__set_function_array(TH_function *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__functions.array = fray;
  TH__functions.len = len;
  TH__functions.index = 0;
}

void TH__expose_function(const __FlashStringHelper *name, TH_funptr fptr){
  assert_return(TH__functions.index <= TH__functions.len);
  assert_return(flash_len(name) <= TH_MAX_NAME_LEN);
  assert_raise_return(TH__functions.array, ERR_VALUE); // assert not null
  uint8_t len = flash_len(name);
  assert_raise_return(len > 0, ERR_VALUE);

  TH__functions.array[TH__functions.index].el.name = name;
  TH__functions.array[TH__functions.index].el.name_len = len;
  TH__functions.array[TH__functions.index].fptr = fptr;
  sdebug(F("Added F:")); cdebug(name); cdebug(F(" len:"));
  edebug(TH__functions.array[TH__functions.index].el.name_len);
  TH__functions.index++;
}

thread *UI__expose_thread(const __FlashStringHelper *name, TH_thfunptr fptr){
  debug(F("ExpFun"));
  assert_return(TH__threads.index <= TH__threads.len, NULL);
  assert_return(flash_len(name) <= TH_MAX_NAME_LEN, NULL);
  assert_raise_return(TH__threads.array, ERR_VALUE, NULL); // assert not null

  uint8_t len = flash_len(name);
  assert_raise_return(len > 0, ERR_VALUE, NULL);

  TH__threads.array[TH__threads.index].el.name = name;
  TH__threads.array[TH__threads.index].el.name_len = len;
  TH__threads.array[TH__threads.index].fptr = fptr;
  TH__threads.array[TH__threads.index].pt.lc = PT_INNACTIVE;
  sdebug(F("Added T:")); cdebug(name); cdebug(F(" len:"));
  cdebug(TH__threads.array[TH__threads.index].el.name_len);
  TH__threads.index++;
  cdebug(F(" Index:")); edebug(TH__threads.index);
  return &(TH__threads.array[TH__threads.index - 1]);
}

// Thread Array
void TH__set_thread_array(thread *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__threads.array = fray;
  TH__threads.len = len;
  for(uint8_t i = 0; i < len; i++){
    TH__threads.array[i].fptr = NULL;
    TH__threads.array[i].pt.lc = PT_INNACTIVE;
  }
}

// #####################################################
// ### Internal Functions

void log_thread_exit(thread *th){
  L_print(F("[Th Exit]:"));
  L_println(th->el.name);
}

void log_thread_start(thread *th){
  L_print(F("[Th Start]:"));
  L_println(th->el.name);
  debug(th->pt.lc);
}

unsigned short function_exists(TH_thfunptr fun){
  for(uint8_t i = 0; i < TH__threads.index; i++){
    assert_raise_return(TH__threads.array[i].fptr != fun, 
    ERR_VALUE, true);
  }
  return false;
}

void th_set_innactive(thread *f){
  sdebug(F("Set Th InA:"));
  edebug(f->el.name);
  f->pt.lc = PT_INNACTIVE;
}

// schedule an initialized function
// returns 0 on error
uint8_t schedule_thread(thread *fun, char *input) {
  uint8_t out = false;
  uint8_t len;
  len = flash_len(fun->el.name);
  th_calling = (char *)malloc(len + 1);
  memcheck(th_calling);
  flash_to_str(fun->el.name, th_calling);
  
  sdebug(F("Calling:"));
  edebug(fun->el.name);
  assert_raise(fun->pt.lc == PT_INNACTIVE, ERR_VALUE, 0);
  PT_INIT(&(fun->pt));
  assert(fun->fptr);
  out = fun->fptr(&(fun->pt), input);
  if(out >= PT_EXITED){
    th_set_innactive(fun);
    out = true;
  }
  else{
    log_thread_start(fun);
    out = true;
  }
error:
  free(th_calling);
  th_calling = NULL;
  return out;
}


// #####################################################
// ### Exported Functions

thread *TH_get_thread(char *name){
  thread *th;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  sdebug(F("gt: "));
  
  for(i = 0; i < TH__threads.index; i++){
    th = &TH__threads.array[i];
    if(cmp_str_elptr(name, name_len, th)){
      edebug(th->el.name);
      return th;
    }
  }
  edebug("NotFound");
  return NULL;
}

TH_function *TH_get_function(char *name){
  TH_function *fun;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  sdebug(F("gf: "));
  
  for(i = 0; i < TH__functions.index; i++){
    fun = &TH__functions.array[i];
    if(cmp_str_elptr(name, name_len, fun)){
      edebug(fun->el.name);
      return fun;
    }
  }
  edebug("NotFound");
  return NULL;
}

uint8_t call_thread(char *name, char *input){
  thread *th = TH_get_thread(name);
  if(th == NULL) return false;
  schedule_thread(th, input);
  return true;
}

uint8_t call_function(char *name, char *input){
  TH_function *fun = TH_get_function(name);
  if(fun == NULL) return false;
  th_calling = name;
  fun->fptr(input);
  th_calling = NULL;
  return true;
}

uint8_t call_name(char *name, char *input){
  if(call_thread(name, input) == false) {
    return call_function(name, input);
  }
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

void kill_thread(thread *th){
  th->pt.lc = PT_KILL_VALUE;
}

void kill_thread(char *name){
  kill_thread(TH_get_thread(name));
}

void kill_thread(const __FlashStringHelper *name){
  char *cname = (char *)malloc(flash_len(name));
  flash_to_str(name, cname); 
  kill_thread(cname);
  free(cname);
}

uint8_t restart_thread(thread *th){
  assert_raisem_return(th->pt.lc == PT_INNACTIVE, ERR_VALUE, th->el.name, 0);
  schedule_thread(th, EH_EMPTY_STR);
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
        th_set_innactive(th);
        log_thread_exit(th);
      }
error:
      clrerr();
      PT_YIELD(  (ptsmall *)(&pt));
    }
  }
  PT_END(  (ptsmall *)(&pt));
}


// *****************************************************************************
// **************  Small Threads

void TH__set_thread_array(sthread *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__sthreads.array = fray;
  TH__sthreads.len = len;
  for(uint8_t i = 0; i < len; i++){
    TH__sthreads.array[i].fptr = NULL;
    TH__sthreads.array[i].pt.lc = PT_INNACTIVE;
  }
}

sthread *TH__expose_thread(uint8_t el_num, TH_thfunptr fptr){
  debug(F("ExpSTh"));
  assert_return(TH__sthreads.index <= TH__sthreads.len, NULL);
  assert_raise_return(TH__sthreads.array, ERR_VALUE, NULL); // assert not null

  TH__sthreads.array[TH__sthreads.index].el_num = el_num;
  TH__sthreads.array[TH__sthreads.index].fptr = fptr;
  TH__sthreads.array[TH__sthreads.index].pt.lc = PT_INNACTIVE;
  sdebug(F("Added T:")); cdebug(el_num); cdebug(F(" len:"));
  TH__sthreads.index++;
  cdebug(F(" Index:")); edebug(TH__sthreads.index);
  return &(TH__sthreads.array[TH__sthreads.index - 1]);
}

/*
void log_thread_exit(sthread *th){
  L_print(F("[Th Exit]:"));
  L_println(th->el_num);
}

void log_thread_start(sthread *th){
  L_print(F("[Th Start]:"));
  L_println(th->el_num);
  debug(th->pt.lc);
}
*/

void th_set_innactive(sthread *th){
  sdebug(F("Set Th InA:"));
  edebug(th->el_num);
  th->pt.lc = PT_INNACTIVE;
}

uint8_t schedule_thread(sthread *th, char *input) {
  uint8_t out = false;
  
  sdebug(F("Calling:"));
  edebug(th->el_num);
  assert_raise(th->pt.lc == PT_INNACTIVE, ERR_VALUE, 0);
  PT_INIT(&(th->pt));
  assert(th->fptr);
  out = th->fptr(&(th->pt), EH_EMPTY_STR);
  if(out >= PT_EXITED){
    th_set_innactive(th);
    out = true;
  }
  else{
    //log_thread_start(th);
    out = true;
  }
error:
  return out;
}

sthread *TH_expose_run_thread(uint8_t el_num, TH_thfunptr fun, char *input){
  sdebug(F("Sch Raw:"));
  edebug(el_num);
  clrerr();
  sthread *th = TH__expose_thread(el_num, fun);
  iferr_log_return(NULL);
  sdebug(F("Check name:"));
  cdebug(th->el_num);
  schedule_thread(th, input);
  return th;
}


sthread *TH_expose_run_thread(uint8_t el_num, TH_thfunptr fun){
  return expose_run_thread(el_num, fun, EH_EMPTY_STR);
}

sthread *TH_get_thread(uint8_t el_num){
  uint8_t i;
  sdebug(F("gt: "));
  for(i = 0; i < TH__sthreads.index; i++){
    if(TH__sthreads.array[i].el_num == el_num){
      edebug(TH__sthreads.array[i].el_num);
      return &TH__sthreads.array[i];
    }
  }
  edebug("NotFound");
  return NULL;
}

void kill_thread(sthread *th){
  th->pt.lc = PT_KILL_VALUE;
}

void kill_thread(uint8_t el_num){
  sthread *th = TH_get_thread(el_num);
  assert_raise_return(th, ERR_VALUE);
  kill_thread(th);
}

uint8_t call_thread(uint8_t el_num, char *input){
  sthread *th = TH_get_thread(el_num);
  assert_raise_return(th, ERR_VALUE, false);
  schedule_thread(th, input);
  return true;
}

uint8_t thread_loop(){
  uint16_t init_lc;
  uint8_t fout;
  sthread *th;
  
  // ***** don't try this at home kids ********
  static struct PTsmall pt = {0};
  // So now "pt" will be (ptsmall *)(&pt)
  // ******************************************
  PT_BEGIN((ptsmall *)(&pt));
  while(true){
    PT_YIELD((ptsmall *)(&pt));
    for(th_loop_index = 0; th_loop_index < TH__sthreads.index; th_loop_index++){
      th = &TH__sthreads.array[th_loop_index];
      init_lc = th->pt.lc;
      if(init_lc == PT_INNACTIVE) continue;
      assert(th->fptr, th_loop_index);    // NOT NULL PTR
      
      fout = th->fptr(&(th->pt), EH_EMPTY_STR);

      if((fout >= PT_EXITED) || (init_lc == PT_KILL_VALUE)){
        if(fout < PT_EXITED){
          seterr(ERR_TYPE);
          log_err(F("NoDie"));
        }
        th_set_innactive(th);
        //log_thread_exit(th);
      }
error:
      clrerr();
      PT_YIELD((ptsmall *)(&pt));
    }
  }
  PT_END((ptsmall *)(&pt));
}




