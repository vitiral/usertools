/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * User Interface library to make microcontroller threading integreate into a simple
 *  user interface that allows the calling of functions and threads as well as the 
 *  query of variables.
 * See UserGuide_ui.html for more information.
 */
 
 
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "ui.h"
#include "threading.h"
#include "strtools.h"
#include "MemoryFree.h"

#include <avr/wdt.h>
#include <avr/interrupt.h>

#define LEDPIN 2


// #####################################################
// ### Globals

#define MAX_STR_LEN 25

uint16_t ui_loop_time = 0;

const __FlashStringHelper **UI__thread_names = NULL;
const __FlashStringHelper **UI__function_names = NULL;
const __FlashStringHelper **UI__variable_names = NULL;

TH_VariableArray UI__variables = {0, 0, 0};
UI_FunctionArray UI__functions = {0, 0, 0};

void UI__expose_variable(void *varptr, uint8_t varsize){
  debug(F("ExpV"));
  assert_return(UI__variables.index < UI__variables.len, NULL);
  assert_raise_return(UI__variables.array, ERR_VALUE, NULL); // assert not null

  UI__variables.array[UI__variables.index].fptr = fptr;
  UI__variables.array[UI__variables.index].pt.lc = PT_INNACTIVE;
  UI__variables.index++;
  sdebug(F("Added T:")); cdebug(UI__variables.index); edebug(F(" len:"));
  return &(UI__variables.array[UI__variables.index - 1]);
}

UI_function expose_function(TH_funptr fptr){
  debug(F("ExpF"));
  assert_return(UI__functions.index < UI__functions.len, NULL);
  assert_raise_return(UI__functions.array, ERR_VALUE, NULL); // assert not null

  UI__functions.array[UI__functions.index].fptr = fptr;
  UI__functions.array[UI__functions.index].pt.lc = PT_INNACTIVE;
  UI__functions.index++;
  sdebug(F("Added T:")); cdebug(UI__functions.index); edebug(F(" len:"));
  return &(UI__functions.array[UI__functions.index - 1]);
}

// #####################################################
// ## Helper functions

enum UI_TYPES{
  UI_THREAD_TYPE,
  UI_FUNCTION_TYPE,
  UI_VARIABLE_TYPE
};

void *get_object(char *name, uint8_t type){
  uint8_t i, el_num;
  const __FlashStringHelper **type_names;
  
  sdebug(F("gt: "));
  clrerr();

  uint8_t len;
  switch(type){
    case UI_THREAD_TYPE:
      len = TH__threads.index;
      break;
    case UI_FUNCTION_TYPE:
      len = UI__functions.index;
      break;
    case UI_VARIABLE_TYPE:
      len = UI__variables.index;
      break;
    default:
      assert(0);
  }
    
  // check if name is a number
  el_num = get_int(name);
  if(not errno){
    edebug();
    
    assert_raise(el_num < len, ERR_INDEX);
    switch(type){
    case UI_THREAD_TYPE:
      return (void *)get_thread(el_num);
    case UI_FUNCTION_TYPE:
      return (void *)get_function(el_num);
    case UI_VARIABLE_TYPE:
      return (void *)get_variable(el_num);
    default:
      assert(0);
    }
  }
  
  clrerr();
  switch(type){
  case UI_THREAD_TYPE:
    type_names = UI__thread_names;
  case UI_FUNCTION_TYPE:
    type_names = UI__function_names;
  case UI_VARIABLE_TYPE:
    type_names = UI__variable_names;
  default:
    assert(0);
  }
  assert_raise(type_names, ERR_INPUT); // not null
  
  for(i = 0; i < len; i++){
    if(cmp_str_flash(name, type_names[i])){
      edebug(type_names[i]);
      switch(type){
      case UI_THREAD_TYPE:
        return &TH__threads.array[i];
      case UI_FUNCTION_TYPE:
        return &UI__functions.array[i];
      case UI_VARIABLE_TYPE:
        return &UI__variables.array[i];
      }
    }
  }
  edebug();
  raise(ERR_INPUT);
error:
  return NULL;
}

// Get from index
TH_variable *get_variable(uint8_t el_num){
  assert_raise_return(el_num < UI__variables.len, ERR_INDEX, NULL);
  return &UI__variables.array[el_num];
}

UI_function *get_function(uint8_t el_num){
  assert_raise_return(el_num < UI__functions.len, ERR_INDEX, NULL);
  return &UI__functions.array[el_num];
}

// Get from name
thread *get_thread(char *name){
  return (thread *)get_object(name, UI_THREAD_TYPE);
}

UI_function *get_function(char *name){
  return (UI_function *)get_object(name, UI_FUNCTION_TYPE);
}

TH_variable *get_variable(char *name){
  return (TH_variable *)get_object(name, UI_VARIABLE_TYPE);
}

void put_inputs(pthread *pt, char *input){
  // loads the inputs into a protothread.
  uint16_t myint;
  float myfloat;
  char *word;
  
  sdebug(F("PutIn:"));
  while(true){
    clrerr();
    edebug(*input);
    word = get_word(input);
    
    //myfloat = get_float(word);
    //AND DO ERROR CHECKING
    
    myint = get_int(word);
    if(not errno){
      cdebug(myint); cdebug("\t\t");
      pt->put_input(myint);
      continue;
    }
    
    if(errno or (not input or not word)) {
      // no more words left
      clrerr();
      return;
    }
    
    cdebug(word); cdebug("\t\t");
    pt->put_input(word);
  }
}

uint8_t call_function(UI_function *fun, char *input){
  // Can raise error
  pthread pt;
  uint8_t out;
  put_inputs(&pt, input);
  if(errno){
    out = 0;
    goto done;
  }
  out = (*fun->fptr)(&pt);
done:
  pt.clear_data();
  return out;
}

uint8_t call_function(char *name, char *input){
  return call_function(get_function(name), input);
}

void schedule_thread(char *name, char *input){
  uint8_t el_num;
  thread *th;
  char *c;
  
  th = get_thread(name);
  assert_raise_return(th, ERR_INPUT);
  
  put_inputs(&th->pt, input);
  schedule_thread(th);
}

// #####################################################
// ### Interrupts

void ui_watchdog(){
  seterr(ERR_CRITICAL);
  log_err("TO");

  L_print(th_loop_index);
  if(UI__)
  L_println(TH__threads.array[th_loop_index].el.name);

  ui_loop_time = millis();
  //asm volatile ("  jmp 0"); 
  wdt_reset();
  wdt_enable(WDTO_250MS);
}

//Activating watchdog UI
ISR(WDT_vect) {
   ui_watchdog();
}

void ui_wdt_setup(void)
{
    //disable interrupts
    cli();
    //reset watchdog
    wdt_reset();
    //set up WDT interrupt
    WDTCSR = (1<<WDCE)|(1<<WDE);
    //Start watchdog timer with 4s prescaller
    WDTCSR = (1<<WDIE)|(1<<WDE)|(1<<WDP3);
    //wdt_enable(WDTO_2S);  // can NOT use this -- must use custom
    //Enable global interrupts
    sei();
}

void ui_pat_dog(){
  wdt_reset();
  ui_loop_time = millis(); // pet the custom dog.
}

// #####################################################
// ### User Commands

void ui_process_command(char *c){
  char *c2;
  char *word;
  
  sdebug(F("Parse CMD:"));
  edebug(*c);
  c = pass_ws(c);
  c2 = get_word(c);
  iferr_log_return();
  assert_return(c); assert_return(c2);
  sdebug(F("Calling Name:"));
  cdebug(c2); cdebug(':');
  edebug(c);
  
  call_function(c2, c); // name, input
}

void cmd_t(char *input){
  // Calling thread from command
  char *word = get_word(input);
  iferr_log_return();
  schedule_thread(word, input);
}

void cmd_v(char *input){
  // Printing variable from command
  char *word = get_word(input);
  iferr_log_return();
  print_variable(word);
}

void cmd_kill(pthread *pt){
  char *thname = pt->get_str_input(0);
  assert_raise_return(thname, ERR_INPUT);
  sdebug(F("k:")); edebug(thname);
  thread *th = get_thread(thname);
  if(th) {
    // print name
    kill_thread(th);
  }
  else{
    raise_return(ERR_INPUT);
  }
  L_print(F("Killed:"));
  L_println(thname);
}

void print_option_name(const __FlashStringHelper *name){
  L_repeat('*', 5);
  L_print('[');
  L_print(name);
  L_println(']');
}

void print_options(pthread *pt){
  uint8_t i = 0;
  print_option_name(F("f"));
  for(i = 0; i < UI__functions.index; i++){
    L_println(UI__function_names[i]);
  }
  
  print_option_name(F("t"));
  for(i = 0; i < TH__threads.index; i++){
    L_print(i);
    L_write('\t');
    L_println(UI__thread_names[i]);
  }
  
  print_option_name(F("v"));
  for(i = 0; i < UI__variables.index; i++){
    L_println(UI__variable_names[i]);
  }
}

// #####################################################
// ### UI Loop
PT_THREAD user_interface(pthread *pt){
  uint8_t v;
  static uint8_t i = 0;
  char c;
  static char *buffer;
  
  if(buffer == NULL){
    buffer = (char *)malloc(MAX_STR_LEN);
    memcheck(buffer);
    buffer[0] = 0;
  }
  
  if(L_available()){
    while(L_available()){
      c = L_read();
      buffer[i] = c;
      if(i > MAX_STR_LEN){
        while(L_available()) L_read();
        raise(ERR_COM, F("Size"));
      }
      else if(buffer[i] == UI_CMD_END_CHAR){
        buffer[i + 1] = 0;
        sdebug(F("Command:"));
        edebug(buffer);
        ui_process_command(buffer);
        goto done;
      }
      i += 1;
    }
  }
  return PT_YIELDED;
done:
error:
  memclr(buffer);
  i = 0;
  return PT_YIELDED;
}

void ui_loop(){
  ui_pat_dog();
  L_set_wrote(false);
  thread_loop();
  if(L_wrote){
    L_repeat('#', 5);
    L_println();
  }
}


// #####################################################
// ### Setup
void ui_std_greeting(){
  L_println(F("!!!Make sure you are sending NL + CR\n?=help\n"));
}

void UI__setup_std(){
  debug(F("UiS:"));
  expose_schedule_thread(user_interface);  // user interface. REQUIRED
  
  //expose_thread(system_monitor); // system monitor
  
  expose_function(cmd_t);
  expose_function(cmd_v);
  expose_function(print_options);
  expose_function(cmd_kill);
  
  //ui_watchdog_setup();
}



