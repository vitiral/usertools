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


#define DEBUG
#include <ui.h>           // include protothread library

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
  NUM_THREADS
};

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);
  
  Serial.println("\n\n#########################  Start Setup");
  
  // You must always begin the setup with this call. The inputs are
  // the numbers of exposed:
  // threads, functions, variables
  ui_setup_std(NUM_THREADS, 0, 0);
  
  // This sets the thread names so that you can access them more easily over the terminal.
  // You can abstain from doing this (it will save some space in program memory) BUT if you call them, you need to have the right number.
  // For instance, it would be bad if I only made an name for "led1".
  set_thread_names(F("led1"), F("led2"));
  default_function_names_only();  // this lets us use the default user interface to schedule threads, kill threads, etc. USE ONLY IF YOU HAVE NO FUNCTIONS
  
  // Expose the things we want to access.
  // Make sure you expose things in the correct order.
  expose_thread(blinky_thread);
  expose_thread(blinky_thread);
  
  thread *th;
  
  // See example pt_led_example2
  th = get_thread(LED1);
  th->pt.put_input(1000);
  th = get_thread(LED2);
  th->pt.put_input(900);
  
  schedule_thread(LED1);
  schedule_thread(LED2);
  
  // You must always end the setup with this call
  ui_end_setup();
}


void loop() {
  // ui_loop handles everything for you -- including calling the threading loop, processing user input, and killing threads.
  ui_loop();
}


