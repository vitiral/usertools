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


uint8_t cmd_print_options(pthread *pt);

// #####################################################
// ### Globals

#define MAX_STR_LEN 25

const __FlashStringHelper **UI__thread_names = NULL;
const __FlashStringHelper **UI__function_names = NULL;
const __FlashStringHelper **UI__variable_names = NULL;

UI_VariableArray UI__variables = {0, 0, 0};
UI_FunctionArray UI__functions = {0, 0, 0};

uint8_t fun_calling = 255;

// *******************************
// **** Macro Helpers

void UI__set_variable_array(UI_variable *vray, uint16_t len){
  assert_raise_return(len < TH__MAX_ARRAY_LEN, ERR_VALUE);
  UI__variables.array = vray;
  UI__variables.len = len;
}

void UI__set_function_array(UI_function *fray, uint16_t len){
  assert_raise_return(len < TH__MAX_ARRAY_LEN, ERR_VALUE);
  UI__functions.array = fray;
  UI__functions.len = len;
  for(uint8_t i = 0; i < len; i++){
    UI__functions.array[i].fptr = NULL;
  }
}

UI_variable *UI__expose_variable(void *varptr, uint8_t varsize){
  debug(F("ExpV"));
  assert_return(UI__variables.index < UI__variables.len, NULL);
  assert_raise_return(UI__variables.array, ERR_VALUE, NULL); // assert not null
  
  UI__variables.array[UI__variables.index].vptr = varptr;
  UI__variables.array[UI__variables.index].size = varsize;
  UI__variables.index++;
  sdebug(F("v:")); edebug(UI__variables.index);
  return &(UI__variables.array[UI__variables.index - 1]);
}

UI_function *expose_function(TH_funptr fptr){
  debug(F("ExpF"));
  assert_return(UI__functions.index < UI__functions.len, NULL);
  assert_raise_return(UI__functions.array, ERR_VALUE, NULL); // assert not null

  UI__functions.array[UI__functions.index].fptr = fptr;
  UI__functions.index++;
  sdebug(F("f:")); edebug(UI__functions.index);
  return &(UI__functions.array[UI__functions.index - 1]);
}

// #####################################################
// ## Helper functions

typedef enum UI_TYPES{
  UI_THREAD_TYPE,
  UI_FUNCTION_TYPE,
  UI_VARIABLE_TYPE
};

typedef void *(*GET_OBJECT)(uint8_t index);

// Get from index
UI_variable *get_variable(uint8_t el_num){
  assert_raise_return(el_num < UI__variables.len, ERR_INDEX, NULL);
  return &UI__variables.array[el_num];
}

UI_function *get_function(uint8_t el_num){
  sdebug("Gf:"); edebug(el_num);
  assert_raise_return(el_num < UI__functions.index, ERR_INDEX, NULL);
  return &UI__functions.array[el_num];
}

// Get from name

void *get_object(char *name, uint8_t len, 
    const __FlashStringHelper **type_names, void *objarray, uint8_t size){
  
  uint8_t i;

  sdebug(F("Gobj:")); edebug(name);
  clrerr();
  
  // check if name is a number
  TRY(i = get_int(name));
  debug(i);
  CATCH_ALL{}
  else{
    assert_raise(i < len, ERR_INDEX);
    //return getobj(i);
    sdebug("elnum:"); edebug(i);
    return (void *)((char *)objarray + i*size);
  }
  
  clrerr();
  
  assert_raise(type_names, ERR_INPUT); // not null
  
  for(i = 0; i < len; i++){
    sdebug("i="); edebug(i);
    debug(type_names[i]);
    if(cmp_str_flash(name, type_names[i])){
      edebug(type_names[i]);
      return (void *)((uint8_t *)objarray + i*size);
    }
  }
  raise(ERR_INPUT);
error:
  return NULL;
}

thread *get_thread(char *name){
  return (thread *)get_object(name, TH__threads.index, UI__thread_names, TH__threads.array, sizeof(thread));
}

UI_function *get_function(char *name){
  return (UI_function *)get_object(name, UI__functions.index, UI__function_names, UI__functions.array, sizeof(UI_function));
}

UI_variable *get_variable(char *name){
  return (UI_variable *)get_object(name, UI__variables.index, UI__variable_names, UI__variables.array, sizeof(UI_variable));
}

uint8_t get_index(UI_function *fun){
  //sdebug((uint16_t)fun); cdebug(" "); edebug((uint16_t)UI__functions.array);
  return ((uint8_t *)fun - (uint8_t *)UI__functions.array) / sizeof(UI_function);
}

void put_inputs(pthread *pt, char *input){
  // loads the inputs into a protothread.
  uint16_t myint;
  float myfloat;
  char *word;
  assert_return(input);
  
  sdebug(F("PutIn:")); edebug(input);
  while(true){
    clrerr();
    
    debug("gw");
    word = get_word(input);
    
    if(errno) {
      // no more words left
      clrerr();
      edebug();
      return;
    }
    
    //myfloat = get_float(word);
    //AND DO ERROR CHECKING
    
    debug("gi");
    TRY(myint = get_int(word));
    CATCH_ALL{} // clear errors
    else{
      cdebug("Pi:"); cdebug(myint); cdebug("\t\t");
      pt->put_input(myint);
      continue;
    }
    
    cdebug("Pw:"); cdebug(word); cdebug("\t\t");
    pt->put_input(word);
  }
  edebug("None");
}

uint8_t call_function(UI_function *fun, char *input){
  // Can raise error
  //pthread *pt = (pthread *)malloc(sizeof(pthread));
  pthread pt;
  uint8_t out = 0;
  assert(pt.data == NULL);
  sdebug("PT_DATA="); edebug((uint16_t) pt.data);
  put_inputs(&pt, input);
  pt.print();
  sdebug("PT_DATA="); edebug((uint16_t) pt.data);
  if(errno){
    out = 0;
    goto done;
  }
  sdebug("CF:"); edebug(get_index(fun));
  fun_calling = get_index(fun);
  out = (*fun->fptr)(&pt);
  fun_calling = 255;
error:
done:
  pt.clear_data();
  //memclr(pt);
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

uint8_t print_variable(UI_variable *var){
  int8_t n;
  uint8_t i;
  L_print(F("=0x"));
  for(n = var->size - 1; n >= 0; n--){
    if(((uint8_t *)var->vptr)[n] < 0x10) L_write('0');
    L_print(((uint8_t *)var->vptr)[n], HEX);
  }
  L_println();
  return true;
}

void ui_process_command(char *input){
  char *word;
  
  sdebug(F("Parse CMD:")); edebug(input);
  Serial.println(input);
  word = get_word(input);
  iferr_log_return();
  assert_return(input); assert_return(word);
  sdebug(F("Calling Name:"));
  cdebug(word); cdebug(':'); edebug(input);
  
  call_function(word, input); // name, input
}

// #####################################################
// ### Interrupts

void ui_watchdog(){
  //L_repeat('\n', 4);
  //L_repeat('!', 10); L_write('\t'); L_println(F("TO"));
  
  L_print(F("[TOUT]\nTH:"));
  L_print(th_loop_index);
  L_print("\tF:");
  L_print(fun_calling);
  //L_write('\t');
  //if(UI__thread_names){
  //  L_print(UI__thread_names[th_loop_index]);
  //}
  L_println();

  wdt_reset();
  //wdt_enable(WDTO_250MS);
  wdt_enable(WDTO_500MS);
}

void ui_wdt_setup(void)
{
    //disable interrupts
    cli();
    //reset watchdog
    wdt_reset();
    //set up WDT interrupt
    WDTCSR = (1<<WDCE)|(1<<WDE);
    //Start watchdog timer with 1s prescaller
    WDTCSR = (1<<WDIE)|(1<<WDE)|(1<<WDP3); //(1<<WDP1)|(1<<WDP2);
    //wdt_enable(WDTO_2S);  // can NOT use this -- must use custom
    //Enable global interrupts
    sei();
}


/*
//Activating watchdog UI
ISR(WDT_vect) {
   ui_watchdog();
}
*/


void ui_pat_dog(){
  wdt_reset();
}

// #####################################################
// ### User Commands



uint8_t cmd_t(pthread *pt){
  uint8_t i;
  char *thname = NULL;
  uint8_t type;
  thread *th = NULL;
  // Calling thread from command
  pt->print();
  sdebug(F("T:"));
  assert(not errno);
  i = pt->get_type_input(0);
  iferr_log_catch(); // index error
  if(i <= vt_maxint){
    i = pt->get_int_input(0);
    edebug(i);
    th = get_thread(i);
  }
  else if(i == vt_str){
    char *thname = pt->get_str_input(0);
    edebug(thname);
    th = get_thread(thname);
  }
  else raise(ERR_TYPE);
  iferr_log_catch();
  
  sdebug(F("Ti:")); edebug(get_index(th));
  debug(th->pt.lc);
  assert_raise_return(not is_active(th), ERR_VALUE, 0);
  th->pt.clear_data();
  i = 0;
  for(i = 0;;i++){
    sdebug(F("in:")); cdebug(i); cdebug('\t');
    TRY(type = pt->get_type_input(i));
    CATCH_ALL{
      if(i == 0){
        raise(ERR_INPUT);
      }
      edebug(F("BREAK"));
      break;
    }
    if(i == 0){ // first iteration is the thread itself
      if(type < vt_maxint) th = get_thread(pt->get_int_input(i));
      else th = get_thread(pt->get_str_input(i));
      cdebug("th:"); edebug(get_index(th)); 
    }
    else{
      if(type < vt_maxint) {
        th->pt.put_input((int16_t)pt->get_int_input(i));
        edebug(th->pt.get_int_input(i-1));
      }
      else if (type == vt_str) {
        th->pt.put_input(pt->get_str_input(i));
        edebug(th->pt.get_str_input(i-1));
      }
      else assert(0);
    }
  }
  debug(F("Sch Thread"));
  schedule_thread(th);
  return 1;
error:
  return 0;
}

uint8_t cmd_v(pthread *pt){
  // Printing variable from command
  UI_variable *var;
  int32_t vint;
  int8_t i = 0;
  
  if(pt->get_type_input(0) < vt_maxint){
    var = get_variable(pt->get_int_input(0));
  }
  else{
    var = get_variable(pt->get_str_input(0));
  }
  iferr_log_catch();
  vint = pt->get_int_input(1);
  if(not errno){
    for(i = 1; i <= var->size; i++){
      // move the values at the end onto the variable. Truncate rest.
      ((uint8_t *)var->vptr)[var->size - i] = ((uint8_t *)vint)[sizeof(vint) - i];
    }
  }
  // always print the variable when done
  print_variable(var);
  return 1;
error:
  return 0;
}

uint8_t cmd_kill(pthread *pt){
  debug(F("UI_kill"));
  char *thname = pt->get_str_input(0);
  thread *th;
  assert_raise(thname, ERR_INPUT);
  slog_info(F("k:")); elog_info(thname);
  th = get_thread(thname);
  if(th) {
    // print name
    kill_thread(th);
  }
  else{
    raise(ERR_INPUT);
  }
  L_print(F("Killed:"));
  L_println(thname);
  return 1;
error:
  return 0;
}

void print_option_name(const __FlashStringHelper *name){
  L_repeat('*', 5);
  L_print('[');
  L_print(name);
  L_println(']');
}

#define PRINT_IGNORE PT_KILL_VALUE - 1

void print_option_line(uint8_t i, const __FlashStringHelper **name_array, uint16_t pt_line){
  L_print(i);
  if(name_array){
    L_write('\t');
    L_print(name_array[i]);
  }
  if(pt_line != PRINT_IGNORE){
    L_repeat('\t', 2);
    L_print(pt_line);
  }
  L_println();
}

uint8_t cmd_print_options(pthread *pt){
  ui_print_options();
  return 1;
}

void ui_print_options(){
  uint8_t i = 0;
  print_option_name(F("f"));
  for(i = 0; i < UI__functions.index; i++){
    print_option_line(i, UI__function_names, PRINT_IGNORE);
  }
  
  print_option_name(F("t"));
  for(i = 0; i < TH__threads.index; i++){
    print_option_line(i, UI__thread_names, TH__threads.array[i].pt.lc);
  }
  
  print_option_name(F("v"));
  for(i = 0; i < UI__variables.index; i++){
    print_option_line(i, UI__variable_names, PRINT_IGNORE);
  }
}

// #####################################################
// ### UI Loop
PT_THREAD user_interface(pthread *pt){
  uint8_t v;
  static uint8_t i = 0;
  char c;
  static char *buffer;
  
  PT_BEGIN(pt);
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
  PT_END(pt);
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
  
  expose_function(cmd_print_options);
  expose_function(cmd_t);
  expose_function(cmd_v);
  expose_function(cmd_kill);
  
  cmd_print_options((pthread *) 10);
  
  //ui_wdt_setup();
}



