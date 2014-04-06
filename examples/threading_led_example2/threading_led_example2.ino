/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate how to use the threading module.
 */


#include <MemoryFree.h>
#include <threading.h>           // include protothread library

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

enum MYTHREADS{
  LED1, 
  LED2, 
  NUM_THREADS
};

void setup() {

  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);

  thread_setup(NUM_THREADS); // it is important that you include the correct number of threads
  
  // Expose the functions we want to schedule as threads
  // Make sure you expose things in the correct order.
  thread *led1 = expose_thread(blinky_thread); //LED1
  thread *led2 = expose_thread(blinky_thread); //LED2

  // All threads have an attached protothread (pt).
  //   They have member functions. This lets us
  //   store inputs on a linked list. (slow access but
  //   takes up very little memory)
  led1->pt.put_input(1000);
  led2->pt.put_input(900);

  // Show that the inputs have been gotten
  Serial.println(F("Inputs:"));
  Serial.println(led1->pt.get_int_input(0));
  Serial.println(led2->pt.get_int_input(0));
  schedule_thread(LED1); 
  schedule_thread(LED2);
  Serial.println(freeMemory()); // prints out the amount of memory that is free. It is light weight!
}

void loop() {
  thread_loop(); // this will call scheduled threads one at a time, passing in their pt objects.
}


