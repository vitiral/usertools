/* Copyright (c) 2014, Garrett Berg
 * This example code is released into the public domain
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate the ui.h and threading.h
 * libraries
 * 
 */

//#include <Stream.h>

#define DEBUG  // if commented out, DEBUG statements and other logs go away

#include <SoftwareSerial.h>  // BUG: must be included before errorhandling.h
#include <errorhandling.h>

#include <ui.h>

// Simple function we are going to call from user interface
void hello_world(char *input){
  Logger.print(F("HELLO WORLD: "));
  Logger.println(input);
}

// Does a hangup to show that the Arduino automatically records
//  where the timeout failure was and then restarts
void hangup(char *c){
  Logger.println("hanging...");
  while(1);
}

// Threads must also recieve a protothread pointer (pthread)
// These they can use with the protothread library to keep track
// of where they are between calls.
uint8_t print_back(pthread *pt, char *input){
  static char saved[100];  // character array to store input, there are better ways to do this
  static uint16_t time;        // keep track of time for waiting
  
  PT_BEGIN(pt);  // from pt library
  strncpy(saved, input, 99); //returns the word. Input goes to 

  PT_YIELD(pt); // future calls will ignore the input. You can reset this thread by killing it
  Logger.println(F("Print Back Starting"));
  while(1){
    Logger.print(F("Your phrase:"));
    Logger.println(saved);
    PT_WAIT_MS(pt, time, 5000);  // automatically uses the "time" variable to do a wait.
    //   note that time has to be static!
  }
  PT_END(pt);
}

unsigned int answer = 0x42;

void setup(){
  uint16_t mem;
  Serial.begin(57600);  
  debug(F("Debug active"));
  ui_setup_std(1, 2, 1);
  
  // make variables and functions available to the user interface
  expose_variable("answer", answer);
  expose_function("hi", hello_world);
  expose_function("hang", hangup);
  expose_thread("print", print_back);

  Logger.println("Your options are:");
  
  call_name("mon", "");     // start the system monitor -- will print every 5 seconds
                            // turn off with "kill mon" in the terminal
                            // turn back on with "mon" in the terminal
  ui_std_greeting();        // print standard greeting.
}

void loop(){
  ui_loop();
}
