/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate how to use the threading module.
 */


#include <MemoryFree.h>   // demonstrates how lightweight this is
#include <threading.h>    // main modules

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


// This is where you declare which functions are threads.
// Here we use the same function twice (we will configure
// Them differently below).
expose_threads(TH_T(blinky_thread), TH_T(blinky_thread));

// We use enum to keep track of threads.
enum MYTHREADS{
  LED1, 
  LED2
};

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);
  
  Serial.println("MEM");
  Serial.println(freeMemory()); // prints out the amount of memory that is free. It is light weight!
  
  // Call this before you call any other threading operations.
  setup_threading(50);  // 50 == amount of dynamically alocated memory that pt uses. 
                        // Every time you put data into a pthread, it uses this memory block.
                        // See UserGuide_ReMem for more details
  
  Serial.println(freeMemory()); // prints out the amount of memory that is free. It is light weight!

  // You can get the protothread component from any thread and put data into it as shown.
  pthread *th;
  th = get_thread(LED1);
  th->put_input(1000);
  th = get_thread(LED2);
  th->put_input(900);

  // Scheduling the thread means it will run.
  schedule_thread(LED1);
  schedule_thread(LED2);
  
}

void loop() {
  thread_loop(); // this will call scheduled threads one at a time, passing in their pt objects.
}


