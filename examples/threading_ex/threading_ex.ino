/* Copyright (c) 2014, Garrett Berg
 * This example code is released into the public domain
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate the ui.h and threading.h
 * libraries
 * 
 * See setup() for how to create new threads, make variables
 * and functions accessible to the user.
 *
 * start the terminal at 57600 baud to be able to toggle led's,
 * read led variable values, "record" data.
 *   -- Be sure to type "mon" to see the system monitor in action 
 */

#include <Stream.h>

#define DEBUG  // if commented out, DEBUG statements and other logs go away

#include <SoftwareSerial.h>  // BUG: must be included before errorhandling.h
#include <errorhandling.h>

#include <ui.h>

#define LEDPIN 13
short led_value = 0; // we are going to be able to access this through the terminal

void _toggle(){ // used by all functions to do toggle
  led_value = !digitalRead(LEDPIN); // keep track of led_value
  digitalWrite(LEDPIN, led_value);
}

// Example of a thread
PT_THREAD blinky(pthread *pt, char *input){
  static unsigned int period; // The period can be set.
  static unsigned int time;   // ONLY static variables will keep inside of threads
  
  PT_BEGIN(pt);     // necessary for beggining of pthreads
  period = get_int(input);
  debug(period);
  iferr_log_catch();        // if there is an error, log it and go to "error:"
  
  pinMode(LEDPIN, OUTPUT); // Always initialize after PT_BEGIN (except for static variables)
  
  while(true){ // while loop forever
    _toggle();
    PT_WAIT_MS(pt, time, period);   // causes thread to exit, will be called again by ui_loop().
        // only goes forward after 250 ms
  }
error:    // This is the standard syntax for the errorhandling library. It helps you do final
          // cleanup before ending the thread.
  PT_KILL(pt);      // the task manager can kill this thread. If it does, it will jump here.
                    // do final cleanup here.
  debug("Error or killed");
  pinMode(LEDPIN, INPUT);
  PT_END(pt);        // All pthreads must be ended with this.
}

#define DATA_LEN 20


// Another Thread, call with "record"
// user_input: record
//   records the data of blinky, then exits
PT_THREAD record_led(pthread *pt, char *input){
  // again, everything must be static.
  static unsigned int time;  
  static long unsigned int time_data[DATA_LEN];
  static unsigned short led_data[DATA_LEN];
  static unsigned short i;
  
  PT_BEGIN(pt); 
  debug("Starting record"); // a debug statement from errorhandling.h
  time = millis();
  for(i=0; i < DATA_LEN; i++){
    led_data[i] = digitalRead(LEDPIN);           // note: you can access led_value later
    time_data[i] = millis(); // record period. 
    PT_WAIT_MS(pt, time, 100);
  }
  // Print out the data into a table
  Logger.println("LED Data");
  uint8_t column_widths[]= {4, 10, 10};
  for(i=0; i < DATA_LEN; i++){
     print_row(String(i) + String('\t') +          // first column, separated by '\t'
              String(led_data[i]) + String('\t') + // second column
              String(time_data[i]),                // final column
              column_widths);                      // input our column widths, make sure they have the same len as all the '\t'
   }
   debug("Ending record");
   PT_END(pt);
}

void toggle_led(char *c){ // the user will have access to this one
  pinMode(LEDPIN, OUTPUT); // force it to output
  _toggle();
}

void setup(){
  Serial.begin(57600);
  ui_setup_std(1, 1, 2); // necessary call early on. Input = variables, functions, threads (see below)

  expose_variable("led", led_value);      // acess led by typing "v led"
  expose_function("toggle", toggle_led);  // call function by typing "toggle"
  expose_thread("record", record_led);    // activate thread by typing "record"
  
  expose_thread("blink", blinky);          // start thread automatically, can be reactiavted by typing "blink"
  call_name("blink", "250");               // just as if the user did it.
  
  log_info("Started");  // logging, similar to "debug" statement
  Logger.println("  ***  Welcome to the Threading Example. Here are your options:");
  print_options("");    // print out the options for the user right away. Note that we have to pass an empty
                        // string because this function is just like our exposed functions
  EH_test();
}

void loop(){
  ui_loop();  // automatically calls threads and does user interface
}



