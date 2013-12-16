/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate new features to protothreads
 * PT_LOCAL_BEGIN ::
 *   starts a protothread locally, does not need to be passed
 *   and does not need to be initialized.
 *   Can only be one thread running at a time.
 *   use pt_local as the protothread.
 *
 * PT_WAIT_MS(pt) ::
 *   Blocks theread until the specified number of ms have 
 *   passed.
 */

// This example uses ONLY protothreads, to give you an idea of how they
// work on their own.
// It also uses the PT_BEGIN_LOCAL function so you don't have to declare
// your own protothreads
#include <pt.h>   // include protothread library

#define LEDPIN 13  // LEDPIN is a constant 

//static struct pt pt1, pt2; // each protothread needs one of these

void setup() {
  pinMode(LEDPIN, OUTPUT); // LED init
}

void toggleLED() {
  boolean ledstate = digitalRead(LEDPIN); // get LED state
  ledstate ^= 1;   // toggle LED state using xor
  digitalWrite(LEDPIN, ledstate); // write inversed state back
}

/* This function toggles the LED after 'interval' ms passed */
static int protothread1(int interval) {
  static uint16_t time; // needs to be static because the function is left every run
  PT_LOCAL_BEGIN(pt);
  while(1) {
    //* Use PT_WAIT_MS instead of the timestamp, etc. 
    PT_WAIT_MS(pt, time, interval); // macro to do "wait for milliseconds"
    toggleLED();
  }
  PT_END(pt);
}

/* exactly the same as the protothread1 function */
static int protothread2(int interval) {
  static uint16_t time;
  PT_LOCAL_BEGIN(pt);
  while(1) {
    PT_WAIT_MS(pt, time, interval);
    toggleLED();
  }
  PT_END(pt);
}

void loop() {
  protothread1(900); // schedule the two protothreads
  protothread2(1000); // by calling them infinitely
}

