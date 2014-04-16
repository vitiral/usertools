/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate how to use more features of the user interface.
 * - access to variabels (sub_time -- subtracts an amount of time from all led periods)
 * - access to functions (reinit -- reinitializes everything)
 *
 * In the terminal, make sure your line ending is "Both NL & CR" and type:
 *  "?" -- gives you a list of options.
 *  "k ***" -- will kill thread ***. I.e. "k led1" will make it so that only 1 thread is blinking (blinks at period of 900ms)
 *  "t led1 600" -- If you just killed led1, this will restart led1 with a period of 600ms
 *  "v sub_time 500" -- you will see the led's blink much faster (500ms is being taken off both periods)
 *  "v sub_time"     -- get the value of sub_time in hex
 *  "reinit"         -- reinitializes everything.
 */

#include <ui.h>           // include protothread library
#include <MemoryFree.h>

#define LEDPIN 13  // LEDPIN is a constant 

uint16_t sub_time = 0;  // subtracts time from LED periods

void toggleLED() {
  boolean ledstate = digitalRead(LEDPIN); // get LED state
  ledstate ^= 1;   // toggle LED state using xor
  digitalWrite(LEDPIN, ledstate); // write inversed state back
}

/* This function toggles the LED after 'interval' ms passed */
PT_THREAD blinky_thread(pthread *pt) {
  // Notice that this function doesn't require any static integers. Everything
  // is stored inside of th
  PT_BEGIN(pt);
  while(1) {
    //* Use PT_WAIT_MS instead of the timestamp, etc.
    PT_WAIT_MS(pt, pt->get_int_input(0) - sub_time); // macro to do "wait for milliseconds"
    iferr_log();
    toggleLED();  
  }
  PT_END(pt);
}

enum MYTHREADS{
  LED1, 
  LED2
};

enum MYFUNCS{
  REINIT
};

enum MYVARS{
  SUB_TIME
};

// This function can be called form the command line as "reinit"
// It resets all the settings.
uint8_t reinit(pthread *pt){
  pthread *th;
  // First we try to kill the threads.
  // Note: TRY simply silences any error outputs. You still have to clear errors
  // afterwards (if you don't clear errors, it can effect the next funciton call)
  // Note: this also destroys all thread data.
  TRY(set_thread_innactive(get_thread(LED1)));  
  clrerr();
  TRY(set_thread_innactive(get_thread(LED2)));
  clrerr();
  // Note: setting threads innactive is not a good idea for threads 
  // that need to be killed gracefully. If your thread needs to do
  // cleanup, use kill_thread and wait 1 loop cycle to re-schedule.
  
  sub_time = 0;
  // All threads have an attached protothread (pt).
  //   They have member functions. This lets us
  //   store inputs on a linked list. (slow access
  //   takes up very little memory)
  th = get_thread(LED1);
  th->put_input(1000);
  iferr_log_return(0);
  th = get_thread(LED2);
  th->put_input(900);
  iferr_log_return(0);
  
  schedule_thread(LED1);
  schedule_thread(LED2);
}

// Expose the things we want to access.
// Threads always need to be exposed if you want to schedule them
// Note: the order is important!
  
expose_threads(TH_T(blinky_thread), TH_T(blinky_thread));  // wrap threads in TH_T
expose_functions(UI_F(reinit));    // Wrap functions in UI_F
UI_V(v1, sub_time);              // Variables have to be declared specially. Declare variable names first with UI_V
//UI_V(v2, othervar) -- if you had more variables, continue in this way
expose_variables(UI_VA(v1));     // Then wrap the variable names in UI_VA. Alot of things have to be done to take up
                                 // zero RAM!

// The names have to be done similarily to variables
UI_STR(tn1, "led1");
UI_STR(tn2, "led2");
expose_thread_names(tn1, tn2);  // no wrapping required here.

UI_STR(fn1, "reinit");
expose_function_names(fn1);

UI_STR(vn1, "sub");
expose_variable_names(vn1);

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);
  
  Serial.println("\n\n#########################  Start Setup");
  
  // This makes the names actually accessible to the threading module.
  set_thread_names();
  set_function_names();
  set_variable_names();
  
  // this must be called before any threading (except setting names)
  setup_ui(200);  // 200 == the amount of dynamic memory our threads can use. 
                  // I recommend at least 100, more if you use alot of strings
  
  // This has two purposes: it initilizes the values during setup, and it can be called
  // by the user to reinitilize the values.
  reinit(NULL); // Note: NULL is only used because the function doesn't actually use it's pthread input.
  
}

void loop() {
  static uint32_t time;
  ui_loop();
  
  if(millis() - time > 2000){
    Serial.print("fM:"); Serial.println(freeMemory());
    time = millis();
  }
  
}


