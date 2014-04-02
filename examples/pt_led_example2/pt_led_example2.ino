/* Copyright (c) 2014, Garrett Berg
 * Original Copyright (c) 2011, Jan Clement
 * licenced under the GPL
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate new features to protothreads
 * 
 * pthread instead of "struct pt"
 * 
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

#define DEBUG
#include <SoftwareSerial.h>
#include <threading.h>           // include protothread library

#define LEDPIN 13  // LEDPIN is a constant 

//static struct pt pt1, pt2; // each protothread needs one of these

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
  PT1, 
  PT2, 
  NUM_THREADS
};

void setup() {
  //PROGMEM const __FlashStringHelper *TH__THREAD_NAMES[] = {F("HELLO"), F("YOU 2")}; // use this form

  //set_thread_names(F("HELLO"), F("YOU 2"));
  //Serial.println(TH__thread_names[0]);
  //Serial.println(TH__thread_names[1]);

  pinMode(LEDPIN, OUTPUT); // LED init
  Serial.begin(57600);

  thread_setup(NUM_THREADS);
  thread *pt1 = expose_thread(blinky_thread);
  thread *pt2 = expose_thread(blinky_thread);

  // All threads have an attached protothread (pt).
  //   They have member functions. This lets us
  //   store inputs on a linked list. (slow access
  //   takes up very little memory)
  pt1->pt.put_input(1000);
  pt2->pt.put_input(900);

  // Show that the inputs have been gotten
  Serial.println(F("Inputs:"));
  Serial.println(pt1->pt.get_int_input(0));
  Serial.println(pt2->pt.get_int_input(0));
  schedule_thread(PT1);
  schedule_thread(PT2);
}

void loop() {
  thread_loop();
}


