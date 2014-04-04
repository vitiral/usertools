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


#define DEBUG
#include <ui.h>           // include protothread library

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
  // is stored inside of pt.
  PT_BEGIN(pt);
  while(1) {
    //* Use PT_WAIT_MS instead of the timestamp, etc.
    PT_WAIT_MS(pt, pt->get_int_input(0) - sub_time); // macro to do "wait for milliseconds"
    toggleLED();
  }
  PT_END(pt);
}

enum MYTHREADS{
  LED1, 
  LED2, 
  NUM_THREADS
};

enum MYFUNCS{
  REINIT,
  NUM_FUNCS
};

enum MYVARS{
  SUB_TIME,
  NUM_VARS
};

// This function can be called form the command line as "reinit"
// It resets all the settings.
uint8_t reinit(pthread *pt){
  thread *th;
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
  th->pt.put_input(1000);
  th = get_thread(LED2);
  th->pt.put_input(900);
  
  schedule_thread(LED1);
  schedule_thread(LED2);
}

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);
  
  Serial.println("\n\n#########################  Start Setup");
  
  // You must always begin the setup with this call. The inputs are
  // the numbers of exposed:
  // threads, functions, variables
  ui_setup_std(NUM_THREADS, NUM_FUNCS, NUM_VARS);
  
  set_thread_names(F("led1"), F("led2"));
  set_function_names(F("reinit"));
  set_variable_names(F("sub"));
  
  // Expose the things we want to access.
  // Threads always need to be exposed if you want to schedule them
  expose_thread(blinky_thread);
  expose_thread(blinky_thread);
  
  expose_function(reinit);
  expose_variable(sub_time);
  
  // This function doesn't actually use it's input, but it still needs it to be added to the ui
  // Here it has two purposes: it initilizes the values during setup, and it can be called
  // by the user to reinitilize the values.
  reinit(NULL); 
  
  // You must always end the setup with this call
  ui_end_setup();
}

void loop() {
  static uint32_t time = millis();
  ui_loop();
  
}


