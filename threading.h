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


// #####################################################
// ### Initialization Macros
extern pthread *TH__threads;
extern uint8_t TH__threads_len;
extern uint8_t th_loop_index;

typedef struct TH_thread_funptr{
  TH_funptr fptr;   // must always be first!
};

void TH__setup_threads(const TH_thread_funptr *thfptrs);

#define TH_T(T)   {&T}
#define expose_threads(...)  PROGMEM const TH_thread_funptr _TH__THREAD_FUNPTRS[] = {__VA_ARGS__, NULL}
#define setup_threading(MEM_SIZE) do{setup_pt(MEM_SIZE); PT__RM.print(); TH__setup_threads(_TH__THREAD_FUNPTRS);}while(0)

// Eventually I want to use this method:
// 
// This will make it so that:
//  - threads will take up half the memory
//  - functions and variables in ui will take up NO MEMORY.

//#define thread_function(name, func)  do{static thread name; schedule_thread(name, func)}while(0) 


uint8_t thread_loop();

// #####################################################
// ### Package Access

pthread *get_thread(uint8_t el_num);
uint8_t get_index(pthread *th);
uint8_t thread_exists(pthread *th);

uint8_t schedule_thread(pthread *th);
uint8_t schedule_thread(uint8_t el_num);

void kill_thread(pthread *th);
void kill_thread(uint8_t el_num);

uint8_t is_active(pthread *th);
void set_thread_innactive(pthread *f);



#endif
