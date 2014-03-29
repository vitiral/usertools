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

// 6 bytes
typedef struct TH_variable {
  void        *vptr;
  uint8_t     size;
};

// 5 bytes
typedef struct TH_function{
  TH_funptr      fptr;
};

// 7 bytes
typedef struct thread{
  TH_thfunptr     fptr;
  pthread         pt;
};

typedef struct TH_fake_thread{
  TH_thfunptr     fptr;
  PTnorm          pt;
};

typedef struct TH_VariableArray{
  TH_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct TH_FunctionArray{
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
// ### Initialization Macros

#define TH_MAX_NAME_LEN 10

#define set_thread_names(...)    do{  \
        PROGMEM const __FlashStringHelper *TH__THREAD_NAMES[] = {__VA_ARGS__};     \
        TH__thread_names = TH__THREAD_NAMES;                                       \
      }while(0)

extern const __FlashStringHelper **TH__thread_names;

// do funcitons and variables


// Setup Macros
// w/o user interface
#define thread_setup(T) do{                                         \
      static TH_fake_thread TH__thread_array[T];                        \
      TH__set_thread_array((thread *) TH__thread_array, T);  \
    }while(0)

// for user interface, with names etc.
#define thread_setup_ui(V, F, T) do{                                       \
      static TH_variable TH__variable_array[V];                         \
      TH__set_variable_array(TH__variable_array, V);                    \
                                                                        \
      static TH_function TH__function_array[F];                         \
      TH__set_function_array(TH__function_array, F);                    \
                                                                        \
      static TH_fake_thread TH__thread_array[T];                       \
      TH__set_thread_array((thread *) TH__thread_array, T);           \
    }while(0)

//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 

// Expose Macros
#define expose_variable(var) TH__expose_variable((void *)&(var), sizeof(var)) 
#define expose_function(FUN) TH__expose_function(FUN)

uint8_t thread_ui_loop();
uint8_t thread_loop();

// #####################################################
// ### Macro Helpers

void TH__set_variable_array(TH_variable *vray, uint16_t len);

void TH__set_function_array(TH_function *fray, uint16_t len);

void TH__set_thread_array(thread *fray, uint16_t len);



// #####################################################
// ### User Functions

TH_variable *get_variable(uint8_t el_num);

TH_function *get_function(uint8_t el_num);

void call_function(uint8_t el_num, char *input);

// #####################################################
// ### Package Access

//void TH__expose_variable(void *varptr, uint8_t varsize);
//void TH__expose_function(TH_funptr fptr);
thread *expose_thread(TH_thfunptr fptr);

thread *expose_run_thread(TH_thfunptr fun, char *input);
thread *expose_run_thread(TH_thfunptr fun);

thread *get_thread(uint8_t el_num);

uint8_t schedule_thread(thread *th, char *input);
uint8_t schedule_thread(uint8_t el_num, char *input);

void kill_thread(thread *th);
void kill_thread(uint8_t el_num);

void set_thread_innactive(thread *f);

extern TH_VariableArray TH__variables;
extern TH_FunctionArray TH__functions;
extern TH_ThreadArray TH__threads;

extern uint8_t th_calling;
extern uint8_t th_loop_index;



#endif
