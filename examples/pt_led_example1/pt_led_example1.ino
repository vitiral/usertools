/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Updated by Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate the use of protothreads 
 * in an arduino sketch. It toggles an LED  using two 
 * independent protothreads. One pt toggles every 
 * 1000ms, the other one every 900ms. The result is an 
 * erratic blinking pattern.
 */


// This example document was kept in so that you can see
// more internals to the protothreads. This calls them 
// directly in a similar way to the thread_loop in
// threading.

#include <pt.h>   // include protothread library

#define LEDPIN 13  // LEDPIN is a constant 

static pthread pt1, pt2; // creating each pthread object

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
  setup_pt(0);  // declares the maximum amount of memory the pthreads will use (these don't use any).
  PT_INIT(&pt1);  // initialise the two
  PT_INIT(&pt2);  // protothread variables
}

void toggleLED() {
  boolean ledstate = digitalRead(LEDPIN); // get LED state
  ledstate ^= 1;   // toggle LED state using xor
  digitalWrite(LEDPIN, ledstate); // write inversed state back
}

/* This function toggles the LED after 'interval' ms passed */
static int protothread1(pthread *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) { // never stop 
    /* each time the function is called the second boolean
    *  argument "millis() - timestamp > interval" is re-evaluated
    *  and if false the function exits after that. */
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis(); // take a new timestamp
    toggleLED();
  }
  PT_END(pt);
}
/* exactly the same as the protothread1 function */
static int protothread2(pthread *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();
    toggleLED();
  }
  PT_END(pt);
}

void loop() {
  protothread1(&pt1, 900); // schedule the two protothreads
  protothread2(&pt2, 1000); // by calling them infinitely
}
