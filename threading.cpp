
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"
#include "pt.h"

#define DEBUG
#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "serialui.h"
#include "threading.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256


// #####################################################
// ### Struct Declaration

typedef uint8_t (*ui_funptr)(thread *pt, char *input);

class ui_thread
{
public:
  ui_thread(ui_function *func);
  ui_thread();
  ui_function             *function;
};

ui_thread::ui_thread(ui_function *func){
  function = func;
}

ui_thread::ui_thread(){
  function = NULL;
}

typedef struct Variables{
  ui_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct Functions{
  struct ui_function *array;
  uint8_t len;
  uint16_t index;
};


// #####################################################
// ### Globals
Variables variables = {
  0, 0, 0};
Functions functions = {
  0, 0, 0};
LinkedList<ui_thread> threads = LinkedList<ui_thread>();


// #####################################################
// ### Macro Helpers
/*
void UI__init_threads(thr){
 //LinkedList<ui_function> *threads = new LinkedList<ui_function>();
 //LinkedList<int> *threads = new LinkedList<int>();
 threads = thr;
 }
 */

void UI__set_variable_array(ui_variable *vray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  variables.array = vray;
  variables.len = len;
  variables.index = 0;
}

void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize){
  assert_return(variables.index <= variables.len);
  assert_return(fstr_len(name) <= MAX_STR_LEN);
  assert_raise_return(variables.array, ERR_PTR); // assert not null
  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE);

  variables.array[variables.index].el.name = name;
  variables.array[variables.index].el.name_len = len;
  variables.array[variables.index].vptr = varptr;
  variables.array[variables.index].size = varsize;
  debug("Added V");
  debug(name);
  debug(String("len:") + String(variables.array[variables.index].el.name_len));
  variables.index++;
}

void UI__set_function_array(struct ui_function *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  functions.array = fray;
  functions.len = len;
}

ui_function *UI__expose_function(const __FlashStringHelper *name, ui_funptr fptr){
  debug("ExpFun");
  assert_return(functions.index <= functions.len, NULL);
  assert_return(fstr_len(name) <= MAX_STR_LEN, NULL);
  assert_raise_return(functions.array, ERR_PTR, NULL); // assert not null

  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE, NULL);

  functions.array[functions.index].el.name = name;
  functions.array[functions.index].el.name_len = len;
  functions.array[functions.index].fptr = fptr;

  thread pt;
  pt.lc = -1;
  pt.error = 0;
  pt.time = 0;
  functions.array[functions.index].pt = pt;
  debug("Added F");
  debug(name);
  debug(String("len:") + String(len));
  debug(functions.array[functions.index].pt.lc);
  functions.index++;
  return &(functions.array[functions.index - 1]);
}

#define UI_STD_VARLEN 20
#define UI_STD_FUNLEN 20
void ui_setup_std(){
  static unsigned short run = false;
  assert_return(run == false);
  run = true;
  static ui_variable UI__variable_array[UI_STD_VARLEN]; 
  static uint8_t UI__variable_len = UI_STD_VARLEN;
  ui_setup_variables();

  static ui_function UI__function_array[UI_STD_FUNLEN];
  static uint8_t UI__function_len = UI_STD_FUNLEN;
  ui_setup_functions();
}

