
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"
#include "pt.h"

#define DEBUG
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
LinkedList<TH_thread_instance> TH__threads = LinkedList<TH_thread_instance>();


// #####################################################
// ### Macro Helpers
/*_
void UI__init_threads(thr){
 //LinkedList<TH_function> *TH__threads = new LinkedList<TH_function>();
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
  debug("Added V");
  debug(name);
  debug(String("len:") + String(TH__variables.array[TH__variables.index].el.name_len));
  TH__variables.index++;
}

void UI__set_function_array(struct TH_function *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  TH__functions.array = fray;
  TH__functions.len = len;
}

TH_function *UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr){
  debug("ExpFun");
  assert_return(TH__functions.index <= TH__functions.len, NULL);
  assert_return(fstr_len(name) <= MAX_STR_LEN, NULL);
  assert_raise_return(TH__functions.array, ERR_PTR, NULL); // assert not null

  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE, NULL);

  TH__functions.array[TH__functions.index].el.name = name;
  TH__functions.array[TH__functions.index].el.name_len = len;
  TH__functions.array[TH__functions.index].fptr = fptr;

  pthread pt;
  pt.lc = -1;
  pt.error = 0;
  pt.time = 0;
  TH__functions.array[TH__functions.index].pt = pt;
  debug("Added F");
  debug(name);
  debug(String("len:") + String(len));
  debug(TH__functions.array[TH__functions.index].pt.lc);
  TH__functions.index++;
  return &(TH__functions.array[TH__functions.index - 1]);
}

// #####################################################
// ### Internal Functions

unsigned short function_exists(TH_funptr fun){
  for(uint8_t i = 0; i < TH__functions.index; i++){
    assert_raise_return(TH__functions.array[i].fptr != fun, 
    ERR_VALUE, true);
  }
  return false;
}

// schedule an initialized function
// returns 0 on error
uint8_t schedule_function(TH_function *fun, char *input) {
  uint8_t out;
  debug("Calling");
  debug(fun->pt.lc);
  assert_raise_return(fun->pt.lc == -1, ERR_THREAD, 0);
  PT_INIT(&(fun->pt));
  out = fun->fptr(&(fun->pt), input);
  if(out >= PT_EXITED){
    return 1;
  }
  else{
    TH__threads.add(TH_thread_instance(fun));
    return 1;
  }
}


// #####################################################
// ### Exported Functions

// scehdule an uninitilized function
TH_function *schedule_function(const __FlashStringHelper *name, TH_funptr fun){
  debug("Scheduling raw");
  debug(name);
  clrerr();
  TH_function *uifun = UI__expose_function(name, fun);
  iferr_log_return(NULL);
  debug("Check name:");
  debug(uifun->el.name);
  debug(uifun->pt.lc);
  schedule_function(uifun, EH_EMPTY_STR);
  return uifun;
}

uint8_t call_function(char *name, char *input){
  struct TH_function *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);

  debug(String("cf name: ") + String(name));
  for(i = 0; i < TH__variables.index; i++){
    var = &TH__functions.array[i];
    debug(var->el.name);
    if(cmp_str_elptr(name, name_len, var)){
      debug("matched");
      return schedule_function(var, input);
    }
  }
  return 0; 
}

void TH__set_innactive(TH_function *f){
  f->pt.lc = PT_INNACTIVE;
}

void thread_kill(TH_function *f){
  f->pt.lc = PT_KILL;
}

#define UI_STD_VARLEN 20
#define UI_STD_FUNLEN 20
void thread_setup_std(){
  static unsigned short run = false;
  assert_return(run == false);
  run = true;
  static TH_variable UI__variable_array[UI_STD_VARLEN]; 
  static uint8_t UI__variable_len = UI_STD_VARLEN;
  ui_setup_variables();

  static TH_function UI__function_array[UI_STD_FUNLEN];
  static uint8_t UI__function_len = UI_STD_FUNLEN;
  ui_setup_functions();
}






