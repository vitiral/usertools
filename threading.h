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

// 11 bytes
struct TH_fake_thread{  // used so that threads can be declared as static variables
  TH_element      el;
  TH_thfunptr     fptr;
  PTnorm          pt;
  uint16_t        time;  // stores execution time in 10us increments
};

// 7 bytes
typedef struct sthread{
  uint8_t         el_num;
  TH_thfunptr     fptr;
  pthread         pt;
};

typedef struct TH_fake_sthread{
  uint8_t         el_num;
  TH_thfunptr     fptr;
  PTnorm          pt;
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

typedef struct TH_sThreadArray{
  struct sthread *array;
  uint8_t len;
  uint16_t index;
};

// #####################################################
// ### Intended Exported Functions

#define TH_MAX_NAME_LEN 10

// Setup Macros
// w/o user interface
#define thread_setup(T) do{                                         \
      static TH_fake_sthread TH__sthread_array[T];                        \
      TH__set_thread_array((sthread *) TH__sthread_array, T);  \
    }while(0)

// for user interface, with names etc.
#define thread_setup_ui(V, F, T) do{                                       \
      static TH_variable TH__variable_array[V];                         \
      static uint8_t TH__variable_len = V;                              \
      TH__set_variable_array(TH__variable_array, TH__variable_len);     \
                                                                        \
      static TH_function TH__function_array[F];                         \
      static uint8_t TH__function_len = F;                              \
      TH__set_function_array(TH__function_array, TH__function_len);     \
                                                                        \
      static TH_fake_thread TH__thread_array[T];                            \
      static uint8_t TH__thread_len = T;                                    \
      TH__set_thread_array((thread *) TH__thread_array, TH__thread_len);    \
    }while(0)


                                       
//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 

// Expose Macros
#define expose_variable(name, var) TH__expose_variable(F(name), (void *)&(var), sizeof(var)) 
#define expose_function(name, FUN) TH__expose_function(F(name), FUN)
#define expose_thread(name, FUN)   TH__expose_thread(name, FUN)
#define start_thread(name, func) schedule_thread(F(name), func)
#define expose_run_thread(name, func, ...) TH_expose_run_thread(name, func, ##__VA_ARGS__)

uint8_t thread_ui_loop();
uint8_t thread_loop();

void kill_thread(thread *th);
void kill_thread(uint8_t el_num);
void kill_thread(char *name);
void kill_thread(const __FlashStringHelper *name);


// #####################################################
// ### Macro Helpers

void TH__set_variable_array(TH_variable *vray, uint16_t len);
void TH__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);

void TH__set_function_array(TH_function *fray, uint16_t len);
void TH__expose_function(const __FlashStringHelper *name, TH_funptr fptr);

void TH__set_thread_array(thread *fray, uint16_t len);
void TH__set_thread_array(sthread *fray, uint16_t len);

thread *TH__expose_thread(const __FlashStringHelper *name, TH_thfunptr fptr);
sthread *TH__expose_thread(uint8_t el_num, TH_thfunptr fptr);


sthread *TH_expose_run_thread(uint8_t el_num, TH_thfunptr fun, char *input);
sthread *TH_expose_run_thread(uint8_t el_num, TH_thfunptr fun);

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

extern TH_sThreadArray TH__sthreads;

#endif
