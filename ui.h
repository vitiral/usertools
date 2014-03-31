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
typedef struct TH_variable {
  void        *vptr;
  uint8_t     size;
};

// 5 bytes
typedef struct UI_function{
  TH_funptr      fptr;
};

typedef struct TH_VariableArray{
  TH_variable *array;
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
#define thread_setup_ui(V, F, T) do{                                       \
      static TH_variable UI__variable_array[V];                         \
      UI__set_variable_array(UI__variable_array, V);                    \
                                                                        \
      static UI_function UI__function_array[F];                         \
      UI__set_function_array(UI__function_array, F);                    \
                                                                        \
      static TH_fake_thread TH__thread_array[T];                       \
      TH__set_thread_array((thread *) TH__thread_array, T);           \
    }while(0)

      
// ##############################################
// ## Functions and Variables
// Expose Macros

extern TH_VariableArray UI__variables;
extern TH_FunctionArray UI__functions;

void UI__expose_variable(void *varptr, uint8_t varsize);
#define expose_variable(var) UI__expose_variable((void *)&(var), sizeof(var)) 
void expose_function(H_funptr fptr);

void UI__set_variable_array(TH_variable *vray, uint16_t len);
void UI__set_function_array(UI_function *fray, uint16_t len);

TH_variable *get_variable(uint8_t el_num);
UI_function *get_function(uint8_t el_num);

void call_function(uint8_t el_num);


#define UI__MIN_T 1
extern const __FlashStringHelper **UI__thread_names;
#define set_thread_names(...)    do{          \
        PROGMEM const __FlashStringHelper *UI__THREAD_NAMES[] = {F("ui"), __VA_ARGS__};     \
        UI__thread_names = UI__THREAD_NAMES;                                       \
      }while(0)

#define UI__MIN_F 4
extern const __FlashStringHelper **UI__function_names;
#define set_function_names(...)    do{          \
        PROGMEM const __FlashStringHelper *UI__FUNCTION_NAMES[] = \
        {F("t"), F("v"), F("k"), F("?"), __VA_ARGS__};            \
        UI__function_names = UI__FUNCTION_NAMES;                  \
      }while(0)

extern const __FlashStringHelper **UI__variable_names;
#define set_variable_names(...)    do{          \
        PROGMEM const __FlashStringHelper *UI__VARIABLE_NAMES[] = {__VA_ARGS__};     \
        UI__variable_names = UI__VARIABLE_NAMES;                                       \
      }while(0)

#define ui_setup_std(V, F, T) do{                             \
    thread_setup_ui(V, (F) + UI__MIN_F, (T) + UI__MIN_T);     \
    UI__setup_std();                                          \
  }while(0)

void UI__setup_std();
#define UI_CMD_END_CHAR 0x0A
#define UI_CMD_PEND_CHAR 0x0D  // may be right before the end.

#define UI_TABLE_SEP F(" \t ")

// #####################################################
// ### Functions
uint8_t print_variable(char *name);

void ui_watchdog();
void ui_pat_dog();
void system_monitor(char *input);
void monswitch(char *input);
void print_options(char *input);
void ui_loop();
void ui_std_greeting();
#define print_row(S, C) __print_row(&(S), C)
void __print_row(String *row, uint8_t *col_widths);


// #####################################################
// ### Other Module Functions
//void ui_process_command(char *c);
void user_interface();

#endif

