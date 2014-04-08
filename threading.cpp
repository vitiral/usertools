/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Threading library to make microcontroller threading simple, intuitive, and debuggable.
 *   See UserGuide_ui.html in the documentation.
 */
 
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "pt.h"

#include "errorhandling.h"

#include "threading.h"
//#include "strtools.h"
//#include "ui.h"

#define MAX_STR_LEN 100
#define MAX_THREADS 254
// #####################################################
// ### Globals

//TH_ThreadArray TH__threads = {0, 0, 0};
TH_funptr *TH_thread_funptrs = NULL;
pthread *TH__threads = NULL;
uint8_t TH__threads_len = 0;

uint8_t get_len(TH_funptr *thfptrs){
  uint16_t i;
  for(i = 0; thfptrs[i] != NULL; i++){
    debug((uint16_t) thfptrs[i]);
  }
  return i;
}

void TH__setup_threads(const TH_funptr *thfptrs){
  debug((uint16_t) thfptrs[0]);
  int16_t i, n;
  debug(F("Tset:"));
  assert(thfptrs);
  TH_thread_funptrs = (TH_funptr *)thfptrs;
  i = get_len(TH_thread_funptrs);
  sdebug(F("len:")); edebug(i);
  assert(i <= MAX_THREADS);
  TH__threads = (pthread *)malloc(sizeof(pthread) * i);
  memcheck(TH__threads);
  for(n = 0; n < i; n++){
    //PT_INIT(&TH__threads[n]);
    set_thread_innactive(&TH__threads[n]);
  }
  TH__threads_len = i;
  debug(F("TsetD"));
  return;
error:
  memclr(TH__threads);
  return;
}

uint8_t th_loop_index = 255;

// #####################################################
// ### Package Access


uint8_t get_index(pthread *th){
  return ((uint8_t *)th - (uint8_t *)TH__threads) / sizeof(pthread);
}

uint8_t is_active(pthread *th){
  if (th->lc == PT_INNACTIVE) return false;
  else return true;
}

uint8_t thread_exists(pthread *th){
  if(th == NULL) return false;
  uint32_t x = ((uint8_t *)th - (uint8_t *)TH__threads);
  if(x % sizeof(pthread) != 0){
    return false;
  }
  if(x / sizeof(pthread) >= TH__threads_len){//get_len(TH_thread_funptrs)){
    return false;
  }
  return true;
}

void set_thread_innactive(pthread *th){
  th->lc = PT_INNACTIVE;
  th->clear_data();
  //slog_info("Tna:"); elog_info(get_index(th));
}

uint8_t schedule_thread(pthread *th){
  uint8_t out = false;
  
  sdebug(F("schT:")); edebug(get_index(th));
  assert(thread_exists(th));
  assert_raise(not is_active(th), ERR_VALUE, 0);
  debug("Init");
  PT_INIT(th);
  assert(TH_thread_funptrs[get_index(th)]);
  out = TH_thread_funptrs[get_index(th)](th);
  if(out >= PT_EXITED){
    set_thread_innactive(th);
    out = true;
    sdebug("Tns:"); edebug(get_index(th));
  }
  else{
    sdebug("Ts:"); edebug(get_index(th));
    out = true;
  }
error:
  return out;
}

uint8_t schedule_thread(uint8_t el_num){
  pthread *th = get_thread(el_num);
  assert_raise_return(th, ERR_VALUE, false);
  schedule_thread(th);
  return true;
}

pthread *get_thread(uint8_t el_num){
  assert_raise_return(el_num < TH__threads_len, ERR_INDEX, NULL);
  return &TH__threads[el_num];
}

void kill_thread(pthread *th){
  th->lc = PT_KILL_VALUE;
}

void kill_thread(uint8_t el_num){
  pthread *th = get_thread(el_num);
  assert_raise_return(th, ERR_VALUE);
  kill_thread(th);
}

uint8_t thread_loop(){
  uint16_t init_lc;
  uint8_t fout;
  pthread *th;
  
  // ***** don't try this at home kids ********
  static struct PTsmall pt = {0};
  // So now "pt" will be (ptsmall *)(&pt)
  // ******************************************
  PT_BEGIN((ptsmall *)(&pt));
  while(true){
    PT_YIELD((ptsmall *)(&pt));
    for(th_loop_index = 0; TH_thread_funptrs[th_loop_index]; th_loop_index++){
      th = &TH__threads[th_loop_index];
      init_lc = th->lc;
      if(init_lc == PT_INNACTIVE) continue;
      
      fout = TH_thread_funptrs[th_loop_index](th);

      if((fout >= PT_EXITED) || (init_lc == PT_KILL_VALUE)){
        if(fout < PT_EXITED){
          seterr(ERR_TYPE);
          log_err(F("NoDie"));
        }
        set_thread_innactive(th);
      }
error:
      clrerr();
      PT_YIELD((ptsmall *)(&pt));
    }
  }
  PT_END((ptsmall *)(&pt));
}




