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

#define TH__MAX_ARRAY_LEN 256

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

#define expose_threads(...) do{     \
  static PROGMEM TH_funptr TH_THREAD_FUNPTRS[] = {__VA_ARGS__, NULL};
  TH__setup_threads(TH_THREAD_FUNPTRS);
// Eventually I want to use this method:
// 
// This will make it so that:
//  - threads will take up half the memory
//  - functions and variables in ui will take up NO MEMORY.

      
#define expose_threads(...) 
//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 


uint8_t thread_loop();




// #####################################################
// ### Package Access

thread *get_thread(uint8_t el_num);
uint8_t get_index(thread *th);
uint8_t thread_exists(thread *th);

uint8_t schedule_thread(thread *th);
uint8_t schedule_thread(uint8_t el_num);

void kill_thread(thread *th);
void kill_thread(uint8_t el_num);

uint8_t is_active(thread *th);
void set_thread_innactive(thread *f);



#endif
