/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate how to use the basic user interface.
 * 
 * In the terminal, make sure your line ending is "Both NL & CR" and type:
 *  "?" -- gives you a list of options.
 *  "k ***" -- will kill thread ***. I.e. "k led1" will make it so that only 1 thread is blinking (blinks at period of 900ms)
 *  "t led1 600" -- If you just killed led1, this will restart led1 with a period of 600ms
 */

#include <ui.h>           // include protothread library
#include <MemoryFree.h>

#define LEDPIN 13  // LEDPIN is a constant 

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
    PT_WAIT_MS(pt, pt->get_int_input(0)); // macro to do "wait for milliseconds"
    toggleLED();
  }
  PT_END(pt);
}

// These help us keep track of how many threads we have. The order is important!
enum MYTHREADS{
  LED1, 
  LED2, 
};

// This exposes the threads so we can use them. For the user interface, you have to
//  explicitly declare that you won't be using any functions or variables
expose_threads(TH_T(blinky_thread), TH_T(blinky_thread)); // make sure to wrap each thread in TH_T
no_functions();  // if you are not using any functions you have to explicitly declare that
no_variables();  // same for variables

// This declares what our threads will be called. We will now be able to access thread 0 as
// "led1" and thread 1 as "led2". The odd way we have to declare these is because of how
// PROGMEM works.
UI_STR(tn1, "led1");
UI_STR(tn2, "led2");
expose_thread_names(tn1, tn2);
expose_defulat_function_names();    // exposes only the default function names (i.e. ?, k, t, v)


void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);

  Serial.println("\n\n#########################  Start Setup");
  
  // This must be called in your setup function for all exposed names.
  set_thread_names();
  set_function_names();
  
  // This must be called before any threading
  setup_ui(200);  // 200 == the amount of dynamic memory our threads can use. 
                  // I recommend at least 100, more if you use alot of strings
  
  pthread *th;
  
  // See example pt_led_example2
  th = get_thread(LED1);
  th->put_input(1000);
  th = get_thread(LED2);
  th->put_input(900);
  
  schedule_thread(LED1);
  schedule_thread(LED2);
  Serial.println(freeMemory());
}


void loop() {
  // ui_loop handles everything for you -- including calling the threading loop, processing user input, and killing threads.
  ui_loop();
}


