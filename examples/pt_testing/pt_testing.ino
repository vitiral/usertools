
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

//#include <SoftwareSerial.h>
#include <errorhandling.h>
#include <ByteBuffer.h>
#include <logging.h>
#include <MemoryFree.h>
#include <pt.h>

//SoftwareSerial SoftSerial(10, 1); // RX, TX -- 10 should be disconnected (does not receieve)
ByteBuffer failbuffer;

#define test_assert(A) EH_DW(if(!(A)){seterr(ERR_ASSERT); log_err();})

void setup(){
  failbuffer.init(100);
  Serial.begin(57600);
  log_info(F("setup"));
}

PT_THREAD test_simple(pthread *pt, uint16_t *value){
  PT_BEGIN(pt);
  *value = 1;
  PT_YIELD(pt);
  *value = 2;
  PT_END(pt);
}

PT_THREAD test_wait(pthread *pt, uint8_t value){
  PT_BEGIN(pt);
  PT_WAIT_UNTIL(pt, value == 1);
  PT_END(pt);
}

PT_THREAD test_wait_ms(pthread *pt, uint8_t value){
  PT_BEGIN(pt);
  PT_WAIT_MS(pt, value);
  PT_END(pt);
}

PT_THREAD test_spawn(pthread *pt){
  static uint32_t time;
  PT_BEGIN(pt);
  time = millis();
  PT_SPAWN(pt, test_wait_ms, 250);
  time = millis() - time;
  assert_return(245 < time and time < 255, PT_ERROR);
  sdebug(F("Waited:")); edebug(time);
  PT_END(pt);
}

pthread ptstuff;
#define T1_TESTS 5
PT_THREAD test1(pthread *pt_l, unsigned short tp){
  pthread *pt = &ptstuff;
  uint16_t value;
  int32_t test;
  uint16_t mem = freeMemory();
  switch(tp){
  case 0:
    // test simple threading.
  case 1:
    PT_INIT(pt);
    assert(test_simple(pt, &value) == PT_YIELDED);
    assert(value == 1);
    assert(test_simple(pt, &value) == PT_ENDED);
    assert(value == 2);
    break;

  case 2:
    // test wait_while
    PT_INIT(pt);
    assert(test_wait(pt, 0) == PT_WAITING);
    assert(test_wait(pt, 0) == PT_WAITING);
    assert(test_wait(pt, 1) == PT_ENDED);
    break;

  case 3:
    // test wait_ms
    PT_INIT(pt);
    value = millis();
    while(test_wait_ms(pt, 250) == PT_WAITING);
    value = ((uint16_t)millis()) - value;
    debug(value);
    assert(245 < value and value < 255);
    pt->get_int_temp();
    assert(errno == ERR_INDEX);
    clrerr();
    break;

  case 4:
    // test data access
    slog_info(F("Before Clear:")); elog_info(mem);
    pt->put_input((uint8_t)142);
    pt->put_input(-42);
    pt->put_input("hello");
    pt->put_input(-30000);
    slog_info(F("after put:")); clog_info(freeMemory()); clog_info(F(" dif:"));
    elog_info(mem - freeMemory()); 
    assert(pt->get_int_input(0) == 142);
    //sdebug(pt->get_type_input(0));cdebug(" "); cdebug(vt_uint8); cdebug(" "); edebug(vt_maxint);
    assert(pt->get_type_input(0) < vt_maxint);
    
    assert(pt->get_int_input(1) == -42);
    assert(pt->get_type_input(1) < vt_maxint);
    
    assert(strcmp(pt->get_str_input(2), "hello") == 0);
    assert(pt->get_type_input(2) == vt_str);
    
    assert(pt->get_int_input(3) == -30000);
    assert(pt->get_type_input(3) < vt_maxint);
    
    pt->print();
    //Serial.println(F("memtest"));
    //slog_info(F("Mem before:")); clog_info(mem); 
    //clog_info(F(" after:")); elog_info(freeMemory());
    
    // test single point deletions
    pt->del_input(0);
    assert(pt->get_int_input(0) == -42);
    pt->del_input(0);
    assert(strcmp(pt->get_str_input(0), "hello") == 0);
    pt->del_input(0);
    assert(pt->get_int_input(0) == -30000);
    pt->del_input(0);
    assert(mem == freeMemory()); // always make sure there are no memory leaks
    
    pt->put_input(42); //test clear data quick
    pt->clear_data();
    slog_info(F("after clear:")); elog_info(freeMemory());
    break;
    
  case 5:
    PT_INIT(pt);
    value = PT_WAITING;
    while(value == PT_WAITING){
      value = test_spawn(pt);
    }
    iferr_log_catch();
    assert(mem == freeMemory());
    break;
    
  
  }
  assert(mem == freeMemory()); // always make sure there are no memory leaks
  return 0;
error:
  return 1;
}

pthread gpt;
void loop(){
  int out;
  int failures = 0;
  PT_INIT(&gpt);

  Logger.println(F("\n\n\n\n"));
  Logger.println(F("Testing Standard:"));
  for(int n = 0; n <= T1_TESTS; n++){
    Logger.flush();
    Logger.print(F("Testcase:"));
    Logger.println(n);

    clrerr();
    test1(&gpt, n);
    if(errno){
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
  Logger.print(F("Results:  ")); Logger.print(failures); Logger.print(F(" failures out of: ")); 
  Logger.println(T1_TESTS);
  if(failbuffer.getSize()) {
    Logger.println(F("Failed cases:"));
  }

  while(failbuffer.getSize() > 0){
    Logger.print((unsigned short)failbuffer.get());
    Logger.print(F(", "));
  } 
  while(true);
}








