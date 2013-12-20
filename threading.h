/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Threading library to make microcontroller threading simple, intuitive, and debuggable.
 *   See UserGuide_ui.html in the documentation.
 */

#ifndef threading_h
#define threading_h

#include "usertools.h"
#include "pt.h"

// #####################################################
// ### Struct Declaration

typedef uint8_t (*TH_thfunptr)(pthread *pt, char *input);
typedef void (*TH_funptr)(char *input);

// 3 bytes
typedef struct TH_element {
  const __FlashStringHelper   *name;
  uint8_t                     name_len;
};

// 6 bytes
typedef struct TH_variable {
  TH_element  el;
  void        *vptr;
  uint8_t     size;
};

// 5 bytes
typedef struct TH_function{
  TH_element     el;
  TH_funptr      fptr;
};

typedef struct thread{
  TH_element    el;
  TH_thfunptr   fptr;
  pthread       pt;
  uint16_t      time;  // stores execution time in 10us increments
};

struct TH_fake_thread{  // used so that threads can be declared as static variables
  TH_element     el;
  TH_thfunptr    fptr;
  PTnorm         pt;
  uint16_t time;  // stores execution time in 10us increments
};



typedef struct TH_Variables{
  TH_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct TH_Functions{
  TH_function *array;
  uint8_t len;
  uint16_t index;
};

typedef struct TH_ThreadArray{
  struct thread *array;
  uint8_t len;
  uint16_t index;
};

// #####################################################
// ### Intended Exported Functions

#define TH_MAX_NAME_LEN 10

// Setup Macros

#define thread_setup(V, F, T) do{                                       \
      static TH_variable UI__variable_array[V];                         \
      static uint8_t UI__variable_len = V;                              \
      UI__set_variable_array(UI__variable_array, UI__variable_len);     \
                                                                        \
      static TH_function UI__function_array[F];                         \
      static uint8_t UI__function_len = F;                              \
      UI__set_function_array(UI__function_array, UI__function_len);     \
                                                                        \
      static TH_fake_thread UI__thread_array[T];                                \
      static uint8_t UI__thread_len = T;                                \
      UI__set_thread_array((thread *) UI__thread_array, UI__thread_len);           \
    }while(0)
                                       
//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 

// Expose Macros
#define expose_variable(name, var) UI__expose_variable(F(name), (void *)&(var), sizeof(var)) 
#define expose_function(name, FUN) UI__expose_function(F(name), FUN)
#define expose_thread(name, FUN)   UI__expose_thread(F(name), FUN)

#define start_thread(name, func) schedule_thread(F(name), func)

uint8_t thread_loop();

void kill_thread(thread *th);
void kill_thread(uint8_t index);
void kill_thread(char *name);
void kill_thread(const __FlashStringHelper *name);


// #####################################################
// ### Macro Helpers

void UI__set_variable_array(TH_variable *vray, uint16_t len);
void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);

void UI__set_function_array(TH_function *fray, uint16_t len);
void UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr);

void UI__set_thread_array(thread *fray, uint16_t len);
thread *UI__expose_thread(const __FlashStringHelper *name, TH_thfunptr fptr);



// #####################################################
// ### User Functions

thread *schedule_thread(const __FlashStringHelper *name, TH_thfunptr fun);
uint8_t call_thread(char *name, char *input);
uint8_t call_function(char *name, char *input);
uint8_t call_name(char *name, char *input);
uint8_t restart_thread(thread *th);


// #####################################################
// ### Package Access

void TH__set_innactive(thread *f);
extern TH_Variables TH__variables;
extern TH_Functions TH__functions;
extern TH_ThreadArray TH__threads;

TH_variable *TH_get_variable(char *name);
thread *TH_get_thread(char *name);

extern char *th_calling;
extern uint8_t th_loop_index;

#endif
