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

//typedef uint8_t (*TH_thfunptr)(pthread *pt, char *input);
//typedef void (*TH_funptr)(char *input);
typedef uint8_t (*TH_funptr)(pthread *pt);

// 7 bytes
typedef struct thread{
  TH_funptr     fptr;
  pthread         pt;
};

typedef struct TH_fake_thread{
  TH_funptr     fptr;
  PTnorm          pt;
};

typedef struct TH_ThreadArray{
  struct thread *array;
  uint8_t len;
  uint16_t index;
};

// #####################################################
// ### Initialization Macros
extern TH_ThreadArray TH__threads;

extern uint8_t th_loop_index;

#define thread_setup(T) do{                                         \
      static TH_fake_thread TH__thread_array[T];                        \
      TH__set_thread_array((thread *) TH__thread_array, T);  \
    }while(0)

//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 

uint8_t thread_ui_loop();
uint8_t thread_loop();

// #####################################################
// ### Macro Helpers

void TH__set_thread_array(thread *fray, uint16_t len);



// #####################################################
// ### User Functions


// #####################################################
// ### Package Access

thread *expose_thread(TH_funptr fptr);

thread *expose_schedule_thread(TH_funptr fun);
thread *expose_schedule_thread(TH_funptr fun);

thread *get_thread(uint8_t el_num);

uint8_t schedule_thread(thread *th);
uint8_t schedule_thread(uint8_t el_num);

void kill_thread(thread *th);
void kill_thread(uint8_t el_num);

void set_thread_innactive(thread *f);



#endif
