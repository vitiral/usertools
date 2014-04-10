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

/*
#ifndef LOGLEVEL
#ifndef DEBUG
  #define LOGLEVEL 30
#endif
#endif

#if LOGLEVEL < 30
  #define LOGLEVEL 30
#endif
*/

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "logging.h"
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
#define MAX_FUNCTIONS 255
#define MAX_VARIABLES 255
#define MAX_STR_LEN 30

const __FlashStringHelper **UI__thread_names = NULL;
const __FlashStringHelper **UI__function_names = NULL;
const __FlashStringHelper **UI__variable_names = NULL;

UI_function *UI__functions = NULL;
uint8_t fun_calling = 255;
uint8_t UI__functions_len = 0;

UI_variable *UI__variables = NULL;
uint8_t UI__variables_len = 0;

// *******************************
// **** Macro Helpers

UI_function get_function_ptr_obj(uint8_t index){
  //return (UI_function) pgm_read_word((PGM_P) UI__functions + index * sizeof(UI_function));
  
  return (UI_function) {(TH_funptr) get_pointer((PGM_P) UI__functions, index, sizeof(UI_function))};
}

//TH_funptr get_function_ptr_obj(uint8_t index){
  //return (UI_function) pgm_read_word((PGM_P) UI__functions + index * sizeof(UI_function));
//  return (TH_funptr *) pgm_read_word(((PGM_P) UI__functions) + index * sizeof(UI_function));
//}


uint8_t get_len_functions(){
  uint16_t i;
  
  for(i = 0; (get_function_ptr_obj(i).fptr) != NULL; i++);
  return i;
}

void UI__setup_functions(const UI_function *funcs){
  debug((uint16_t) funcs[0].fptr);
  int16_t i;
  debug(F("Fset:"));
  assert(funcs);
  UI__functions = (UI_function *)funcs;
  i = get_len_functions();
  sdebug(F("len:")); edebug(i);
  assert(i <= MAX_FUNCTIONS);
  UI__functions_len = i;
  debug(F("FsetD"));
  return;
error:
  return;
}

UI_variable get_var_pgm(uint8_t index){
  // The data has to be copied from pgm to variable. You then access with the
  // vptr
  
  const UI_variable *var_ptr = (const UI_variable *)get_pointer((PGM_P) UI__variables, index, sizeof(TH_funptr));
  return (UI_variable) {(void *) pgm_read_word((PGM_P) var_ptr),
    (uint8_t) pgm_read_byte(((PGM_P)var_ptr) + 2)};
  
  //return (UI_variable) {(void *) pgm_read_word((PGM_P) UI__variables + index * sizeof(UI_variable)),
  //  (uint8_t) pgm_read_byte((PGM_P) UI__variables + index*sizeof(UI_variable) + 2)};
  
  //return (UI_variable) {(void *) pgm_read_word((PGM_P) UI__variables + index * sizeof(UI_variable)),
  //  (uint8_t) pgm_read_byte((PGM_P) UI__variables + index*sizeof(UI_variable) + 2)};
  
  
  //UI_variable var;
  //PGM_P s = ((PGM_P)UI__variables) + index*sizeof(UI_variable);
  //for(uint8_t i = 0; i < sizeof(UI_variable); i++){
  //  ((uint8_t *)(&var))[i] = pgm_read_byte(s++);
  //}
  //return var;
}

uint8_t get_len_variables(){
  uint16_t i;
  UI_variable var;
  for(i = 0; (var = get_var_pgm(i)).vptr != NULL; i++){
    sdebug((uint16_t)var.vptr); cdebug(' '); edebug(var.size); 
  }
  return i;
}

void UI__setup_variables(const UI_variable **vars){
  int16_t i;
  debug(F("Vset:"));
  assert(vars);
  UI__variables = (UI_variable *)vars;
  
  i = get_len_variables();
  sdebug(F("len:")); edebug(i);
  assert(i <= MAX_VARIABLES);
  UI__variables_len = i;
  debug(F("VsetD"));
  
  return;
error:
  return;
}



// #####################################################
// ## Helper functions

// Get from index
UI_variable get_variable(uint8_t el_num){
  sdebug("Gv:"); edebug(el_num);
  // TODO: RE-ADD THIS
  assert_raise(el_num < UI__variables_len, ERR_INDEX);
error:
  return get_var_pgm(el_num);
}

UI_function get_function(uint8_t el_num){
  sdebug("Gf:"); edebug(el_num);
  assert_raise(el_num < UI__functions_len, ERR_INDEX);
error:
  return get_function_ptr_obj(el_num);
}

// Get from name

uint8_t get_name_index(char *name, uint8_t len, const __FlashStringHelper **type_names){
  
  uint8_t i;

  sdebug(F("Gobj:")); cdebug(name); cdebug(' '); edebug(len);
  clrerr();
  
  assert_raise(type_names, ERR_INPUT); // not null
  
  for(i = 0; i < len; i++){
    sdebug("i="); edebug(i);
    debug(type_names[i]);
    if(cmp_str_flash(name, type_names[i])){
      edebug(type_names[i]);
      return i;
    }
  }
  raise(ERR_INPUT);
error:
  return 0;
}

pthread *get_thread(char *name){
  return get_thread(get_name_index(name, TH__threads_len, UI__thread_names));
}

UI_function get_function(char *name, uint8_t *index){
  *index = get_name_index(name, UI__functions_len, UI__function_names);
  return get_function(*index);
}

UI_variable get_variable(char *name, uint8_t *index){
  *index = get_name_index(name, UI__variables_len, UI__variable_names);
  return get_variable(*index);
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
    
    sdebug("gw:");
    word = get_word(input);
    edebug(word);
    
    if(errno) {
      // no more words left
      clrerr();
      edebug();
      return;
    }
    
    //myfloat = get_float(word);
    //AND DO ERROR CHECKING
    
    TRY(myint = get_int(word));
    CATCH_ALL{} // clear errors
    else{
      cdebug("Pi:"); cdebug(myint); edebug("\t\t");
      pt->put_input(myint);
      continue;
    }
    
    cdebug("Pw:"); cdebug(word); edebug("\t\t");
    pt->put_input(word);
  }
  edebug("None");
}

uint8_t call_function(pthread *pt){
  // Can raise error
  uint8_t out = 0;
  UI_function fun;
  
  uint8_t type = pt->get_type_input(0);
  if(type < vt_maxint) fun = get_function(pt->get_int_input(0));
  else if(type == vt_str) fun = get_function(pt->get_str_input(0), &out);
  else assert(0);
  iferr_log_catch();
  
  pt->del_input(0); // clear index 0, send rest to function
  
  sdebug("CF:"); edebug(out);
  fun_calling = out;
  out = (fun.fptr)(pt);
  fun_calling = 255;
  
error:
done:
  return out;
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

void ui_process_command(char *input, uint8_t endi){
  pthread pt;
  assert(pt.data == NULL);
  input = strip(input, endi);
  sdebug(F("Parse CMD:")); edebug(input);
  put_inputs(&pt, input);
  iferr_log_catch();
  call_function(&pt);
error:
  pt.clear_data();
}

// for debugging
void ui_process_command(char *input){
  return ui_process_command(input, strlen(input));
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



//Activating watchdog UI
//ISR(WDT_vect) {
//   ui_watchdog();
//}



void ui_pat_dog(){
  wdt_reset();
}

// #####################################################
// ### User Commands

// helper function to get thread assuming it is in the first
// input's value
pthread *UI_get_thread(pthread *pt){
  pthread *th;
  uint8_t i = pt->get_type_input(0);
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
  return th;
error:
  return NULL;
}

uint8_t UI_cmd_t(pthread *pt){
  uint8_t i;
  char *thname = NULL;
  uint8_t type;
  pthread *th = NULL;
  debug("CmdT:");
  pt->print();
  
  th = UI_get_thread(pt);
  iferr_log_catch();
  cdebug("th:"); edebug(get_index(th));
  assert(thread_exists(th));
  
  // Prepare thread
  debug(th->lc);
  assert_raise_return(not is_active(th), ERR_VALUE, 0);
  th->clear_data();
  pt->del_input(0); // delete input for transfering
  transfer_inputs(pt, th);
  
  debug(F("Sch Thread"));
  schedule_thread(th);
  return 1;
error:
  return 0;
}

uint8_t UI_cmd_v(pthread *pt){
  // Printing variable from command
  UI_variable var;
  int32_t vint;
  uint8_t type;
  
  type = pt->get_type_input(0);
  iferr_log_catch();
  if(type < vt_maxint){
    var = get_variable(pt->get_int_input(0));
  }
  else if(type == vt_str){
    var = get_variable(pt->get_str_input(0), &type);
  }
  else assert(0);
  sdebug("v:"); edebug(type);
  
  iferr_log_catch();
  TRY(vint = pt->get_int_input(1));
  CATCH_ALL{}
  else{
    assert(sizeof(vint) >= var.size);
//      // this would be for large values first (big endian)
//    for(i = 1; i <= var->size; i++){
//      // move the values at the end onto the variable. Truncate rest.
//      ((uint8_t *)var->vptr)[var->size - i] = ((uint8_t *)vint)[sizeof(vint) - i];
//    }
    
    // Little Endian
    memcpy(var.vptr, &vint, var.size);
  }
  // always print the variable when done
  print_variable(&var);
  return 1;
error:
  return 0;
}

uint8_t UI_cmd_kill(pthread *pt){
  
  debug(F("UI_kill"));
  pt->print();
  pthread *th = UI_get_thread(pt);
  iferr_log_catch();
  
  kill_thread(th);
  L_print(F("Killed:"));
  L_println(get_index(th));
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

void ui_print_options(){
  uint8_t i = 0;
  L_repeat('#', 5); L_print(F(" M:")); L_println(freeMemory());
  print_option_name(F("f"));
  for(i = 0; i < UI__functions_len; i++){
    print_option_line(i, UI__function_names, PRINT_IGNORE);
  }
  
  print_option_name(F("t"));
  for(i = 0; i < TH__threads_len; i++){
    print_option_line(i, UI__thread_names, TH__threads[i].lc);
  }
  
  print_option_name(F("v"));
  for(i = 0; i < UI__variables_len; i++){
    print_option_line(i, UI__variable_names, PRINT_IGNORE);
  }
}

uint8_t UI_cmd_print_options(pthread *pt){
  ui_print_options();
  return 1;
}

// #####################################################
// ### UI Loop
PT_THREAD user_interface(pthread *pt){
  uint8_t v;
  static uint8_t i = 0;
  char c;
  //static char *buffer = NULL; // For some unknown reason, this with malloc doesn't work. TODO
  static char buffer[MAX_STR_LEN];
  
  if(pt->lc == PT_KILLED){
    goto done;
  }
  
  if(L_available()){
    while(L_available()){
      c = L_read();
      buffer[i] = c;
      if(i > MAX_STR_LEN){
        while(L_available()) L_read();
        raise(ERR_COM, F("SIZE"));
      }
      else if(buffer[i] == UI_CMD_END_CHAR){
        sdebug(F("Bp:")); edebug((uint16_t)buffer);
        buffer[i] = 0;
        sdebug(F("Command:"));
        edebug(buffer);
        delay(10);
        ui_process_command(buffer, i);
        goto done;
      }
      i += 1;
    }
  }
  return PT_YIELDED;
done:
error:
  sdebug(F("Bp:")); edebug((uint16_t)buffer);
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
  schedule_thread(TH__threads_len - 1); // ui always guaranteed to be at the end
  ui_print_options();
  
  //ui_wdt_setup();
}

