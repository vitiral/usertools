/* Copyright (c) 2014, Garrett Berg
 * This example code is released into the public domain
 *
 * Author: Garrett Berg <garrett@cloudformdesign.com>
 * 
 * Example code to demonstrate use of the errorhandling
 * library.
 */

#define DEBUG // without this nothing would be printed

#include <SoftwareSerial.h> // BUG: must include before errorhandling.h
#include <errorhandling.h>

short int isint(char c){
  if(('0' <= c) and (c <= '9')) return true;
  else return false;
}

int char_to_int(char c){
  // Makes use of the raise_return format. 
  // note that raisem raises a message too, in this case
  // the character in hex format 
  // Also note that if there is an error, -1 is returned
  assert_raisem_return(isint(c), ERR_TYPE, (int)c, -1); 
  // Note: A simpler way would be:
  // assert_return(isint(c), -1); 
  // However, the error would be ERR_ASSERT instead of
  // ERR_TYPE
  return c - '0';
}

int get_user_int(){
  int outint;
  while(!Serial.available());
  char c = Serial.read();
  debug(String(c, HEX) + String(" ") + String(isint(c), HEX)); // Doing a debug check to check value.
  outint = char_to_int(c);
  debug(String("outint:") + String(outint));  // show the output
  iferr_log_return(-1); // if there is an error, it logs it and returns -1
  return outint;
}

// Top Level Function
void print_user_int(){
  // shows teh use of the "catch" syntax. This is how
  // all non-print error checking works (i.e. assert, assert_raise, etc)
  int user_int = get_user_int(); // expects to get an integer from the user
  iferr_log_catch();  // if there was an error, log it and go to error:
  Serial.print("Output: ");
  Serial.println(user_int);
  return;
error:
  debug("Caught Error");
  clrerr_log(); //clears error flags and logs that it did so.
  return;
}

#define BAUD 57600
void setup(){
  Serial.begin(BAUD);
  log_info(String("Set up at baud: ") + String(BAUD));
  Logger.println("********************");
  Logger.println("MAKE SURE YOU ARE NOT SENDING LINE ENDINGS. Set to 'No Line Ending'\n");
  EH_test();
}

void loop(){
  Serial.println("Input Int:");
  print_user_int();
  return;
}


