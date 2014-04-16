
// It seems that the compiler does some fun stuff with __FILE__ :D :D :D :D
/* Copyright (c) 2014, Garrett Berg
 * This example code is released into the public domain
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Code to test the errorhandling library
 */
// 12452 -> 9968 without debug
#define DEBUG

#include <SoftwareSerial.h>
#include <errorhandling.h>

#include <ByteBuffer.h>
#include <logging.h>
#include <MemoryFree.h>
#include <strtools.h>
#include <pt.h>
#include <ui.h>

SoftwareSerial SoftSerial(10, 1); // RX, TX -- 10 should be disconnected (does not receieve)
ByteBuffer failbuffer;

uint8_t testerr = 0;
#define test_assert(A) EH_DW(if(!(A)){seterr(ERR_CRITICAL); testerr = ERR_CRITICAL; log_err(); clrerr();})

void print_functions(){
  L_println(UI__function_names[0]);
  L_println(UI__function_names[1]);
  L_println(UI__function_names[2]);
}

enum THREADS{
  TH_SET_X1,
  TH_SET_X2,
  TH_MAIN
};

uint8_t x = 0;
uint8_t set_x(pthread *pt){
  x = pt->get_int_input(0);
  L_print("Set x ="); 
  L_println(x);
  return 1;
}


PT_THREAD main_test(pthread *pt){
  static uint8_t i = 0;
  char txt[30];
  //log_info(pt->lc);// clog_info(" "); elog_info(i);
  i++;

  //ui_print_options();
  PT_BEGIN(pt);
  
  PT_YIELD(pt); // initialization yield.
  
  schedule_thread(TH_SET_X1);
  PT_YIELD(pt);
  debug(x);
  test_assert(x==1);

  kill_thread(TH_SET_X1);
  PT_YIELD(pt);
  x = 0;
  PT_YIELD(pt);
  if(x != 0){
    seterr(ERR_CRITICAL); 
    log_err();
  }
  PT_YIELD(pt);

  schedule_thread(TH_SET_X2);
  PT_YIELD(pt);

  debug(x);
  test_assert(x==2);
  kill_thread(TH_SET_X2);
  PT_YIELD(pt);

  debug("using UI");
  // now try with names.
  flash_to_str(F("t 0 4"), txt);
  ui_process_command(txt); // takes over and resets to input = 4
  PT_YIELD(pt);

  test_assert(x==4);

  debug("killing");
  flash_to_str(F("k 0\n"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);
  x = 5; // there should be no more setX threads running
  PT_YIELD(pt); 
  debug("hi");
  test_assert(x == 5);

  flash_to_str(F("t setX1 12"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);

  test_assert(x == 12);
  x = 42;
  PT_YIELD(pt);
  test_assert(x == 12);
  flash_to_str(F("k setX1"), txt);
  ui_process_command(txt);
  
  debug("here");
  PT_YIELD(pt);
  debug("testing ?");
  flash_to_str(F("?"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);

  flash_to_str(F("?"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);

  flash_to_str(F("?"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);

  flash_to_str(F("?"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);

  flash_to_str(F("?"), txt);
  ui_process_command(txt);
  PT_YIELD(pt);
  
  debug("some test");
  flash_to_str(F("t setX1 100\r\n"), txt);
  //waitc();
  ui_process_command(txt);
  
  //waitc();
  
  debug("done?");
  while(true){
    L_println("Main running");
    PT_WAIT_MS(pt, 2000);
  }
  PT_KILL(pt);
  L_println("Killed Successfuly");
  PT_KILLED(pt);
end:
  PT_END(pt);
}


expose_threads(TH_T(set_x), TH_T(set_x), TH_T(main_test));
no_functions();
no_variables();

UI_STR(tn1, "setX1");
UI_STR(tn2, "setX2");
UI_STR(tn3, "Main");
expose_thread_names(tn1, tn2, tn3);
expose_default_function_names();

void setup(){
  pthread *th;
  failbuffer.init(100);
  Serial.begin(57600);
  Serial.println(F("*** SETUP BEGINS ***"));
  
  set_thread_names();
  set_function_names();

  setup_ui(400);
  
  get_thread(TH_SET_X1)->put_input(1);
  get_thread(TH_SET_X2)->put_input(2);
  schedule_thread(TH_MAIN);

  ui_process_command("?");

  log_info(F("all setup"));
}

#define T1_TESTS 100
void loop(){
  int out;
  int failures = 0;

  Logger.println(F("\n\n\n\n"));
  Logger.println(F("Testing Standard:"));
  for(int n = 0; n <= T1_TESTS; n++){
    Logger.flush();
    Logger.print(F("Testcase:"));
    Logger.println(n);
    PT__RM.print();

    clrerr();
    testerr = 0;
    ui_loop();    // MIN UI LOOP
    if(testerr){
      Logger.println(F("*** FAILURE ***"));
      log_err();
      failures++;
      failbuffer.put(n);
      while(1);
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
    ui_loop();    // MIN UI LOOP
  }
}









