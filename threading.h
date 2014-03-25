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
typedef struct sthread{
  TH_thfunptr     fptr;
  pthread         pt;
};

typedef struct TH_fake_sthread{
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
      TH__set_variable_array(TH__variable_array, V);                    \
                                                                        \
      static TH_function TH__function_array[F];                         \
      TH__set_function_array(TH__function_array, F);                    \
                                                                        \
      static TH_fake_sthread TH__sthread_array[T];                       \
      TH__set_thread_array((sthread *) TH__sthread_array, T);           \
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

void TH__set_thread_array(sthread *fray, uint16_t len);



// #####################################################
// ### User Functions

TH_variable *get_variable(uint8_t el_num);

TH_function *get_function(uint8_t el_num);

void call_function(uint8_t el_num, char *input);

// #####################################################
// ### Package Access

//void TH__expose_variable(void *varptr, uint8_t varsize);
//void TH__expose_function(TH_funptr fptr);
sthread *expose_thread(TH_thfunptr fptr);

sthread *expose_run_thread(TH_thfunptr fun, char *input);
sthread *expose_run_thread(TH_thfunptr fun);

sthread *get_thread(uint8_t el_num);

uint8_t schedule_thread(sthread *th, char *input);
uint8_t schedule_thread(uint8_t el_num, char *input);

void kill_thread(sthread *th);
void kill_thread(uint8_t el_num);

void set_thread_innactive(sthread *f);


extern TH_Variables TH__variables;
extern TH_Functions TH__functions;
extern TH_sThreadArray TH__sthreads;

extern uint8_t th_calling;
extern uint8_t th_loop_index;



#endif
