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
 
#ifndef ui_h

#define ui_h

#include "usertools.h"
#include <Arduino.h>
#include "pt.h"
#include "threading.h"
#include "strtools.h"


// 6 bytes
typedef struct UI_variable {
  void        *vptr;
  uint8_t     size;
};

// 5 bytes
typedef struct UI_function{
  TH_funptr      fptr;
};

typedef struct UI_VariableArray{
  UI_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct UI_FunctionArray{
  UI_function *array;
  uint8_t len;
  uint16_t index;
};

// ####################################
// ## GLOBALS

// for user interface, with names etc.
#define thread_setup_ui(T, F, V) do{                                       \
      static UI_variable UI__variable_array[V];                         \
      UI__set_variable_array(UI__variable_array, V);                    \
                                                                        \
      static UI_function UI__function_array[F];                         \
      UI__set_function_array(UI__function_array, F);                    \
                                                                        \
      static TH_fake_thread TH__thread_array[T];                       \
      TH__set_thread_array((thread *) TH__thread_array, T);           \
    }while(0)

      
// Expose Macros

extern UI_VariableArray UI__variables;
extern UI_FunctionArray UI__functions;

UI_variable *UI__expose_variable(void *varptr, uint8_t varsize);
#define expose_variable(var) UI__expose_variable((void *)&(var), sizeof(var)) 
UI_function *expose_function(TH_funptr fptr);

void UI__set_variable_array(UI_variable *vray, uint16_t len);
void UI__set_function_array(UI_function *fray, uint16_t len);

UI_variable *get_variable(uint8_t el_num);
UI_function *get_function(uint8_t el_num);

uint8_t call_function(UI_function *fun, char *input);
uint8_t call_function(char *name, char *input);


#define UI__MIN_T 1
extern const __FlashStringHelper **UI__thread_names;
#define set_thread_names(...)  static const __FlashStringHelper *UI__THREAD_NAMES[] = {__VA_ARGS__, F("ui")};     \
        UI__thread_names = UI__THREAD_NAMES

#define UI__MIN_F 4
extern const __FlashStringHelper **UI__function_names;
#define set_function_names(...) static const __FlashStringHelper *UI__FUNCTION_NAMES[] = \
        {__VA_ARGS__, F("?"), F("t"), F("v"), F("k")};            \
        UI__function_names = UI__FUNCTION_NAMES

#define default_function_names_only() static const __FlashStringHelper *UI__FUNCTION_NAMES[] = \
        {F("?"), F("t"), F("v"), F("k")};             \
        UI__function_names = UI__FUNCTION_NAMES

extern const __FlashStringHelper **UI__variable_names;
#define set_variable_names(...) static const __FlashStringHelper *UI__VARIABLE_NAMES[] = {__VA_ARGS__};     \
        UI__variable_names = UI__VARIABLE_NAMES

#define ui_setup_std(T, F, V) do{                             \
    thread_setup_ui((T) + UI__MIN_T, (F) + UI__MIN_F, V);     \
  }while(0)

#define ui_end_setup() UI__setup_std();


void UI__setup_std();
#define UI_CMD_END_CHAR 0x0A
#define UI_CMD_PEND_CHAR 0x0D  // may be right before the end.

#define UI_TABLE_SEP F(" \t ")

// #####################################################
// ### Used for Testing
void ui_process_command(char *c);
void put_inputs(pthread *pt, char *input);

// #####################################################
// ### Functions
void ui_watchdog();
void ui_pat_dog();
void system_monitor(char *input);
void monswitch(char *input);
void ui_loop();
void ui_std_greeting();
#define print_row(S, C) __print_row(&(S), C)
void __print_row(String *row, uint8_t *col_widths);

void ui_print_options();

// #####################################################
// ### Other Module Functions
void user_interface();

extern volatile uint8_t UI_volatile;

#endif

