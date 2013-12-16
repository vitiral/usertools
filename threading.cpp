
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "pt.h"

#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "threading.h"
#include "strtools.h"
#include "ui.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256



// #####################################################
// ### Globals
TH_Variables TH__variables = {0, 0, 0};
TH_Functions TH__functions = {0, 0, 0};
TH_ThreadArray TH__threads = {0, 0, 0};

// #####################################################
// ### Macro Helpers

// Variable Array
void UI__set_variable_array(TH_variable *vray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__variables.array = vray;
  TH__variables.len = len;
  TH__variables.index = 0;
}

void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize){
  assert_return(TH__variables.index <= TH__variables.len);
  assert_return(fstr_len(name) <= MAX_STR_LEN);
  assert_raise_return(TH__variables.array, ERR_PTR); // assert not null
  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE);

  TH__variables.array[TH__variables.index].el.name = name;
  TH__variables.array[TH__variables.index].el.name_len = len;
  TH__variables.array[TH__variables.index].vptr = varptr;
  TH__variables.array[TH__variables.index].size = varsize;
  sdebug(F("Added V:")); cdebug(name); cdebug(F(" len:"));
  edebug(TH__variables.array[TH__variables.index].el.name_len);
  TH__variables.index++;
}

// Function Array
void UI__set_function_array(TH_function *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__functions.array = fray;
  TH__functions.len = len;
  TH__functions.index = 0;
}

void UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr){
  assert_return(TH__functions.index <= TH__functions.len);
  assert_return(fstr_len(name) <= MAX_STR_LEN);
  assert_raise_return(TH__functions.array, ERR_PTR); // assert not null
  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE);

  TH__functions.array[TH__functions.index].el.name = name;
  TH__functions.array[TH__functions.index].el.name_len = len;
  TH__functions.array[TH__functions.index].fptr = fptr;
  sdebug(F("Added F:")); cdebug(name); cdebug(F(" len:"));
  edebug(TH__functions.array[TH__functions.index].el.name_len);
  TH__functions.index++;
}

// Thread Array
void UI__set_thread_array(thread *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__threads.array = fray;
  TH__threads.len = len;
  for(uint8_t i = 0; i < len; i++){
    TH__threads.array[i].fptr = NULL;
    TH__threads.array[i].pt.lc = PT_INNACTIVE;
  }
}

thread *UI__expose_thread(const __FlashStringHelper *name, TH_thfunptr fptr){
  debug(F("ExpFun"));
  assert_return(TH__threads.index <= TH__threads.len, NULL);
  assert_return(fstr_len(name) <= MAX_STR_LEN, NULL);
  assert_raise_return(TH__threads.array, ERR_PTR, NULL); // assert not null

  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE, NULL);

  TH__threads.array[TH__threads.index].el.name = name;
  TH__threads.array[TH__threads.index].el.name_len = len;
  TH__threads.array[TH__threads.index].fptr = fptr;

  pthread pt;
  pt.lc = PT_INNACTIVE;
  TH__threads.array[TH__threads.index].pt = pt;
  sdebug(F("Added T:")); cdebug(name); cdebug(F(" len:"));
  cdebug(TH__threads.array[TH__threads.index].el.name_len);
  TH__threads.index++;
  cdebug(F(" Index:")); edebug(TH__threads.index);
  return &(TH__threads.array[TH__threads.index - 1]);
}

// #####################################################
// ### Internal Functions

void log_thread_exit(thread *th){
  Serial.print(F("[Th Exit]:"));
  Serial.println(th->el.name);
}

void log_thread_start(thread *th){
  Serial.print(F("[Th Start]:"));
  Serial.println(th->el.name);
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
  uint8_t out;
  sdebug(F("Calling:"));
  edebug(fun->el.name);
  assert_raise_return(fun->pt.lc == PT_INNACTIVE, ERR_THREAD, 0);
  PT_INIT(&(fun->pt));
  assert_return(fun->fptr, 0);
  out = fun->fptr(&(fun->pt), input);
  if(out >= PT_EXITED){
    th_set_innactive(fun);
    return 1;
  }
  else{
    log_thread_start(fun);
    return 1;
  }
}


// #####################################################
// ### Exported Functions

// scehdule an uninitilized function
thread *schedule_thread(const __FlashStringHelper *name, TH_thfunptr fun){
  sdebug(F("Sch Raw:"));
  edebug(name);
  clrerr();
  thread *th = UI__expose_thread(name, fun);
  iferr_log_return(NULL);
  sdebug(F("Check name:"));
  cdebug(th->el.name);
  edebug(th->pt.lc);
  schedule_thread(th, EH_EMPTY_STR);
  return th;
}


uint8_t call_thread(char *name, char *input){
  thread *th;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);

  sdebug(F("cT name:")); cdebug(name); cdebug(F(" index=")); 
  edebug(TH__threads.index);
  for(i = 0; i < TH__threads.index; i++){
    th = &TH__threads.array[i];
    debug(th->el.name);
    if(cmp_str_elptr(name, name_len, th)){
      schedule_thread(th, input);
      return true;
    }
  }
  return false; 
}

uint8_t call_function(char *name, char *input){
  uint8_t name_len = strlen(name);
  TH_function *fun;
  sdebug(F("cT name:")); cdebug(name); cdebug(F(" index=")); 
  edebug(TH__functions.index);
  for(uint8_t i = 0; i < TH__functions.index; i++){
    fun = &TH__functions.array[i];
    if(cmp_str_elptr(name, name_len, fun)){
      fun->fptr(input);
      return true;
    }
  }
  return false;
}

uint8_t ui_call_name(char *name, char *input){
  if(not call_thread(name, input)) return call_function(name, input);
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
    if(cmp_str_elptr(name, name_len, var)){
      return var;
    }
  }
  return NULL;
}

thread *TH_get_thread(char *name){
  thread *th;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  sdebug(F("gt: "));
  edebug(name);
  for(i = 0; i < TH__threads.index; i++){
    th = &TH__threads.array[i];
    if(cmp_str_elptr(name, name_len, th)){
      return th;
    }
  }
  return NULL;
}

void kill_thread(thread *th){
  th->pt.lc = PT_KILL_VALUE;
}

uint8_t restart_thread(thread *th){
  assert_raisem_return(th->pt.lc == PT_INNACTIVE, ERR_THREAD, th->el.name, 0);
  schedule_thread(th, EH_EMPTY_STR);
}

uint8_t thread_loop(){
  static int8_t i = 0;
  uint64_t time;
  uint16_t timeus;
  uint8_t fout;
  uint16_t init_lc;
  thread *th;

  PT_LOCAL_BEGIN(pt);
  while(true){
    PT_YIELD(pt);
    for(i = 0; i < TH__threads.index; i++){
      th = &TH__threads.array[i];
      init_lc = th->pt.lc;
      if(init_lc == PT_INNACTIVE) continue;
      time = millis();
      timeus = micros();
      assert(th->fptr, i);    // NOT NULL PTR
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

      if(fout >= PT_EXITED or init_lc == PT_KILL_VALUE){
        if(fout < PT_EXITED){
          seterr(ERR_THREAD);
          log_err(F("NoDie"));
        }  
        th_set_innactive(th);
        log_thread_exit(th);
        i -= 1;
      }
error:
      clrerr();
      PT_YIELD(pt);
    }
  }
  PT_END(pt);
}



