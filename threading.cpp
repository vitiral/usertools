
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"
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
TH_ThreadArray TH__th_array = {0, 0, 0};
LinkedList<TH_thread_instance> TH__threads = LinkedList<TH_thread_instance>();


// #####################################################
// ### Macro Helpers
/*_
void UI__init_threads(thr){
 //LinkedList<thread> *TH__threads = new LinkedList<thread>();
 //LinkedList<int> *TH__threads = new LinkedList<int>();
 TH__threads = thr;
 }
 */

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

void UI__set_function_array(thread *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__th_array.array = fray;
  TH__th_array.len = len;
}

thread *UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr){
  debug(F("ExpFun"));
  assert_return(TH__th_array.index <= TH__th_array.len, NULL);
  assert_return(fstr_len(name) <= MAX_STR_LEN, NULL);
  assert_raise_return(TH__th_array.array, ERR_PTR, NULL); // assert not null

  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE, NULL);

  TH__th_array.array[TH__th_array.index].el.name = name;
  TH__th_array.array[TH__th_array.index].el.name_len = len;
  TH__th_array.array[TH__th_array.index].fptr = fptr;

  pthread pt;
  pt.lc = -1;
  pt.error = 0;
  pt.time = 0;
  TH__th_array.array[TH__th_array.index].pt = pt;
  sdebug(F("Added F:")); cdebug(name); cdebug(F(" len:"));
  cdebug(TH__th_array.array[TH__th_array.index].el.name_len);
  TH__th_array.index++;
  cdebug(F(" Index:")); edebug(TH__th_array.index);
  return &(TH__th_array.array[TH__th_array.index - 1]);
}

// #####################################################
// ### Internal Functions

void log_thread_exit(thread *th){
  Serial.print(F("[Th Exit]:"));
  Serial.println(th->el.name);
}

void log_thread_start(thread *th){
  Serial.print(F("[Th Start]:"));
  debug();
}


unsigned short function_exists(TH_funptr fun){
  for(uint8_t i = 0; i < TH__th_array.index; i++){
    assert_raise_return(TH__th_array.array[i].fptr != fun, 
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
uint8_t schedule_function(thread *fun, char *input) {
  uint8_t out;
  sdebug(F("Calling:"));
  edebug(fun->el.name);
  assert_raise_return(fun->pt.lc == PT_INNACTIVE, ERR_THREAD, 0);
  PT_INIT(&(fun->pt));
  out = fun->fptr(&(fun->pt), input);
  if(out >= PT_EXITED){
    th_set_innactive(fun);
    return 1;
  }
  else{
    TH__threads.add(TH_thread_instance(fun));
    log_thread_start(fun);
    return 1;
  }
}


// #####################################################
// ### Exported Functions

// scehdule an uninitilized function
thread *schedule_function(const __FlashStringHelper *name, TH_funptr fun){
  sdebug(F("Sch Raw:"));
  edebug(name);
  clrerr();
  thread *th = UI__expose_function(name, fun);
  iferr_log_return(NULL);
  sdebug(F("Check name:"));
  cdebug(th->el.name);
  edebug(th->pt.lc);
  schedule_function(th, EH_EMPTY_STR);
  return th;
}

uint8_t call_thread(char *name, char *input){
  thread *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);

  sdebug(F("cf name:")); cdebug(name); cdebug(F(" index=")); 
  edebug(TH__th_array.index);
  for(i = 0; i < TH__th_array.index; i++){
    var = &TH__th_array.array[i];
    if(cmp_str_elptr(name, name_len, var)){
      return schedule_function(var, input);
    }
  }
  return 0; 
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
  for(i = 0; i < TH__th_array.index; i++){
    th = &TH__th_array.array[i];
    if(cmp_str_elptr(name, name_len, th)){
      return th;
    }
  }
  return NULL;
}

void kill_thread(thread *th){
  th->pt.lc = PT_KILL;
}

uint8_t restart_thread(thread *th){
  assert_raisem_return(th->pt.lc == PT_INNACTIVE, ERR_THREAD, th->el.name, 0);
  schedule_function(th, EH_EMPTY_STR);
}

#define UI_STD_VARLEN 20
#define UI_STD_FUNLEN 20

uint8_t thread_loop(){
  static int8_t i = 0;
  uint64_t time;
  uint16_t timeus;
  uint8_t fout;
  uint16_t kill;
  
  thread *th;
  PT_LOCAL_BEGIN(pt);
  while(true){
    PT_YIELD(pt);
    i = 0;
    while(i < TH__threads.size()){
      th = TH__threads.get(i).th;
      kill = th->pt.lc;
      
      time = millis();
      timeus = micros();
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
      
      if(time > (uint16_t)-1) th->time = -1;
      else th->time = time;
      
      if(fout >= PT_EXITED or kill == PT_KILL){
        if(fout < PT_EXITED){
          seterr(ERR_THREAD);
          log_err(F("NoDie"));
        }  
        th_set_innactive(th);
        TH__threads.remove(i);
        log_thread_exit(th);
        i -= 1;
      }
      clrerr();
      i += 1;
      PT_YIELD(pt);
    }
  }
  PT_END(pt);
}



