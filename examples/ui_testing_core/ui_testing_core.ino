
// It seems that the compiler does some fun stuff with __FILE__ :D :D :D :D
/* Copyright (c) 2014, Garrett Berg
 * This example code is released into the public domain
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Code to test the errorhandling library
 */
// 12452 -> 9968 without debug


#include <Arduino.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define DEBUG

//#include <SoftwareSerial.h>
#include <errorhandling.h>
#include <ByteBuffer.h>
#include <logging.h>
#include <MemoryFree.h>
#include <pt.h>
#include <ui.h>

ByteBuffer failbuffer;

uint8_t testerr = 0;
#define test_assert(A) EH_DW(if(!(A)){seterr(ERR_CRITICAL); testerr = ERR_CRITICAL; log_err(); clrerr();})

void main_test(uint8_t place);

enum THREADS{
  TH_SETX,
  TH_NUM
};

enum FUNCTIONS{
  FUN_SETX,
  FUN_NUM
};

enum VARIABLES{
  VAR_X,
  VAR_NUM
};

uint8_t x = 0;
uint8_t set_x(pthread *pt){
  int16_t newx = pt->get_int_input(0);
  iferr_log_return(0);
  x = newx;
  L_print("Set x ="); 
  L_println(x);
  return PT_YIELDED;
}

void setup(){
  thread *th;
  failbuffer.init(100);
  Serial.begin(57600);
  Serial.println(F("\n\n\n*** SETUP BEGINS ***"));

  ui_setup_std(TH_NUM, FUN_NUM, VAR_NUM);
  set_thread_names(F("setX"));
  set_function_names(F("setX"));
  set_variable_names(F("x"));
  
  expose_thread(set_x);
  expose_function(set_x);
  expose_variable(x);

  ui_end_setup();
  log_info(F("all setup"));
  if(1, 0){
    Serial.println("IT WORKS");
  }
}

void main_test(uint8_t place){
  pthread pt;
  thread *th;
  char txt[30];
  switch(place){
  case 0:
    flash_to_str(F("1 23 hello 4200\r\n"), txt);
    put_inputs(&pt, txt);
    test_assert(pt.get_int_input(0) == 1);
    test_assert(pt.get_int_input(1) == 23);
    test_assert(not strcmp(pt.get_str_input(2), "hello"));
    test_assert(pt.get_int_input(3) == 4200);
    break;

  case 1:
    flash_to_str(F("0 12"), txt); 
    ui_process_command(txt);
    test_assert(x == 12);

    clrerr();
    flash_to_str(F("setX 15"), txt);
    ui_process_command(txt);
    test_assert(x == 15);
    break;
    
  case 2:
     debug("Start Threads");
     th = get_thread(TH_SETX);
     th->pt.clear_data();
     th->pt.put_input(42);
     schedule_thread(TH_SETX);
     debug(x);
     test_assert(x == 42);
     
     set_thread_innactive(th);
     test_assert(th->pt.lc == PT_INNACTIVE);
     break;
     
   case 3:
     debug("ui process threads");
     th = get_thread(TH_SETX);
     flash_to_str(F("t 0 45"), txt);
     ui_process_command(txt);
     test_assert(x == 45);
     debug("killing");
     set_thread_innactive(th);
     test_assert(th->pt.lc == PT_INNACTIVE);
     
     debug(th->pt.lc);
     debug(get_index(th));
     flash_to_str(F("t setX 55"), txt);
     ui_process_command(txt);
     test_assert(x == 55);
     debug("killing");
     set_thread_innactive(th);
     test_assert(th->pt.lc == PT_INNACTIVE);
  
  case 4:
    // '?' is failing alot
    flash_to_str(F("?"), txt);
    ui_process_command(txt);
    break;

  case 5:
    sdebug("should print x: 0x"); edebug(String(x, HEX));
    flash_to_str(F("v x"), txt);
    ui_process_command(txt);
    
    debug("setting x to 0x89");
    flash_to_str(F("v x 0x89"), txt);
    ui_process_command(txt);
    test_assert(x == 0x89);
    
    flash_to_str(F("v x"), txt);
    ui_process_command(txt);
    
   
  }
end:
error:
  pt.clear_data();
  return;
}

#define T1_TESTS 6
void loop(){
  int out;
  int failures = 0;

  Logger.println(F("\n\n\n\n"));
  Logger.println(F("Testing Standard:"));
  for(int n = 0; n < T1_TESTS; n++){
    Logger.flush();
    Logger.print(F("\n\n###################\nTestcase:"));
    Logger.println(n);

    clrerr();
    testerr = 0;
    main_test(n);    // MIN UI LOOP
    if(testerr){
      Logger.println(F("*** FAILURE ***"));
      log_err();
      failures++;
      failbuffer.put(n);
    }
    else{
      Logger.println(F("*** Success ***"));
    }
  }

  Logger.println(F("##### Tests Done #####"));
  Logger.print(F("Results:  ")); 
  Logger.print(failures); 
  Logger.print(F(" failures out of: ")); 
  Logger.println(T1_TESTS);
  if(failbuffer.getSize()) {
    Logger.println(F("Failed cases:"));
  }

  while(failbuffer.getSize() > 0){
    Logger.print((unsigned short)failbuffer.get());
    Logger.print(F(", "  ));
  }

  L_println("!!!! DONE!");
  while(true){
    ui_pat_dog();    // MIN UI LOOP
  }
}










