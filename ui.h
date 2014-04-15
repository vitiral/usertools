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
#include "flash_ptrs.h"


// 6 bytes
typedef struct UI_variable {
  void        *vptr;
  uint8_t     size;
};

// 5 bytes
typedef struct UI_function{
  TH_funptr      fptr;
};

// ####################################
// ## GLOBALS

// ## Threads
#define UI_STD_NUM_TH 1
PT_THREAD user_interface(pthread *pt);
#define expose_threads(...)   PROGMEM const TH_thread_funptr _TH__THREAD_FUNPTRS[]  = {__VA_ARGS__, TH_T(user_interface), NULL}

#define no_threads()          PROGMEM const TH_thread_funptr _TH__THREAD_FUNPTRS[] = {NULL}

// ## Functions
uint8_t UI_cmd_print_options(pthread *pt);
uint8_t UI_cmd_t(pthread *pt);
uint8_t UI_cmd_v(pthread *pt);
uint8_t UI_cmd_kill(pthread *pt);

#define UI_STD_NUM_FUN 4;
#define UI_F(F)   {&(F)}
void UI__setup_functions(const UI_function *thfptrs);
#define expose_functions(...)   PROGMEM const UI_function _UI__FUNCTIONS[] = \
    {__VA_ARGS__, UI_F(UI_cmd_print_options), UI_F(UI_cmd_t), UI_F(UI_cmd_v), \
    UI_F(UI_cmd_kill), {NULL}}
    
#define no_functions()          PROGMEM const UI_function _UI__FUNCTIONS[] = \
    {             UI_F(UI_cmd_print_options), UI_F(UI_cmd_t), UI_F(UI_cmd_v), \
    UI_F(UI_cmd_kill), {NULL}}

// ## Variables
#define UI_V(N, V)   PROGMEM const UI_variable N = {&(V), (uint8_t)sizeof(V)}
#define UI_VA(V)  &(V)
//void UI__setup_variables(const UI_variable *vars);
void UI__setup_variables(const UI_variable **vars);

#define expose_variables(...) PROGMEM const UI_variable _UI__VARIABLE_END = {NULL, (uint8_t) 0}; \
                      PROGMEM const UI_variable *_UI__VARIABLES[] = {__VA_ARGS__, UI_VA(_UI__VARIABLE_END)};

//#define expose_variables(...)  const UI_variable _UI__VARIABLES[] = \
//    {__VA_ARGS__, {NULL, (uint8_t) 0}}

#define no_variables() PROGMEM const UI_variable _UI__VARIABLE_END = {NULL, (uint8_t) 0}; \
                      PROGMEM const UI_variable *_UI__VARIABLES[] = {UI_VA(_UI__VARIABLE_END)};

//#define no_variables() PROGMEM const UI_variable _UI__VARIABLES[] = {{NULL, (uint8_t) 0}}

void UI__setup_std();

// ## Standard Setup
#define setup_ui(MEM_SIZE)        do{       \
    setup_threading(MEM_SIZE);              \
    UI__setup_functions(_UI__FUNCTIONS);    \
    UI__setup_variables(_UI__VARIABLES);    \
    UI__setup_std();                        \
  }while(0)

    
// ## String Access
#define UI_STR(N, D)    PROGMEM const prog_char N[] = D
//#define UI_NAMES(D)     PROGMEM const prog_char *names[] = D

#define UI__MIN_T 1

#define UI_STR(N, D)    PROGMEM const prog_char N[] = D

#define UI__std_thread_names()  UI_STR(UI__THREAD_NAMES_1, "ui")
#define expose_thread_names(...)   UI__std_thread_names();   \
  PROGMEM const prog_char *UI__THREAD_NAMES[] = {__VA_ARGS__, UI__THREAD_NAMES_1, NULL};
//#define default_thread_names_only() 

#define UI__MIN_F 4
#define UI__std_function_names()    UI_STR(UI__FUNCTION_NAMES_1, "?");    \
        UI_STR(UI__FUNCTION_NAMES_2, "t");    \
        UI_STR(UI__FUNCTION_NAMES_3, "v");    \
        UI_STR(UI__FUNCTION_NAMES_4, "k")    
        
#define expose_function_names(...)      UI__std_function_names();                \
        PROGMEM const prog_char *UI__FUNCTION_NAMES[] =                         \
        {__VA_ARGS__, UI__FUNCTION_NAMES_1, UI__FUNCTION_NAMES_2,             \
        UI__FUNCTION_NAMES_3, UI__FUNCTION_NAMES_4, NULL};

#define expose_default_function_names()   UI__std_function_names();             \
        PROGMEM const prog_char *UI__FUNCTION_NAMES[] =                         \
        {UI__FUNCTION_NAMES_1, UI__FUNCTION_NAMES_2,                          \
        UI__FUNCTION_NAMES_3, UI__FUNCTION_NAMES_4, NULL};
        
#define expose_variable_names(...) PROGMEM const prog_char *UI__VARIABLE_NAMES[] = {__VA_ARGS__, NULL};

extern const prog_char **UI__thread_names;
extern const prog_char **UI__function_names;
extern const prog_char **UI__variable_names;

#define set_thread_names()     UI__thread_names = UI__THREAD_NAMES
#define set_function_names()   UI__function_names = UI__FUNCTION_NAMES
#define set_variable_names()   UI__variable_names = UI__VARIABLE_NAMES
#define set_all_names() do{set_thread_names(); set_function_names(); set_variable_names();}while(0)

/*
#define set_variable_names(...)    PROGMEM const prog_char *UI__VARIABLE_NAMES[] = {__VA_ARGS__, NULL}

#define set_thread_names(...)  static const __FlashStringHelper *UI__THREAD_NAMES[] = {__VA_ARGS__, F("ui")};     \
        UI__thread_names = UI__THREAD_NAME

#define set_function_names(...) static const __FlashStringHelper *UI__FUNCTION_NAMES[] = \
        {__VA_ARGS__, F("?"), F("t"), F("v"), F("k")};            \
        UI__function_names = UI__FUNCTION_NAMES

#define default_function_names_only() static const __FlashStringHelper *UI__FUNCTION_NAMES[] = \
        {F("?"), F("t"), F("v"), F("k")};             \
        UI__function_names = UI__FUNCTION_NAMES

extern const __FlashStringHelper **UI__variable_names;
#define set_variable_names(...) static const __FlashStringHelper *UI__VARIABLE_NAMES[] = {__VA_ARGS__};     \
        UI__variable_names = UI__VARIABLE_NAMES
*/

// Access Functions

UI_variable get_variable(uint8_t el_num);
UI_function get_function(uint8_t el_num);

uint8_t call_function(UI_function *fun, char *input);
uint8_t call_function(char *name, char *input);






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

extern volatile uint8_t UI_volatile;

#endif

