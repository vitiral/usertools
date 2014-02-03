/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Library to help with debugging. See user guide in documentation.
 */
 
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "usertools.h"
#include "errorhandling.h"
#include "logging.h"
#include "MemoryFree.h"


#define EH_GEN_END_MSG F(")|")
#define EH_GEN_ST_MSG F(":: ")

uint8_t errprint = 0;
uint8_t loglevel = 50;
uint8_t EH_flush = true;
char *EH_EMPTY_STR = "";

void printmem(){
#if 0
  // all my tests say it takes almost NO time.
  uint16_t mem;
  uint16_t time = micros();
  mem = freeMemory();
  time = micros() - time;
  L_print("Mem:");
  L_print(time);
  L_print("us FM:");
  L_print(mem);
#else
  L_print(F("M:"));
  L_print(freeMemory());
#endif
}
  
void EH_printinfo(char *file, unsigned int line)
{
  L_print(errno); 
  L_write(' '); 
  L_print(file); 
  L_write(':'); 
  L_print(line);
}

void EH_printerrp(){
  L_write('(');
  L_print(errprint);
  L_write(':');
  
  switch(errprint){
  case ERR_NOERR:
    L_print(F("No")); break;
  case ERR_TIMEOUT:
    L_print(F("Timeout")); break;
  case ERR_COM:
    L_print(F("Com")); break;
  case ERR_INPUT:
    L_print(F("Input")); break;
  
  case ERR_TYPE:
    L_print(F("Type")); break;
  case ERR_VALUE:
    L_print(F("Value")); break;
  case ERR_ASSERT:
    L_print(F("Assert")); break;
  case ERR_INDEX:
    L_print(F("Index")); break;
    L_print(F("Mem")); break;
  case ERR_CRITICAL:
    L_print(F("CRIT")); break;
  
  case ERR_CLEARED:
    L_print(F("Clred")); break;
  case ERR_NONEW:
    L_print(F("NoNew")); break;
  case ERR_UNKNOWN:
  default:
    L_print(F("Unknown")); break;
  }
  L_print(F("Err"));
  L_write(')');
}

void EH_start_debug(char *file, unsigned int line){
  L_print(EH_GEN_ST_MSG);
  L_print(F("[DBG]:"));
  printmem();
  L_print(F("(")); 
  EH_printinfo(file, line); 
  L_print(EH_GEN_END_MSG);
}

void EH_start_info(char *file, unsigned int line){
  L_print(EH_GEN_ST_MSG);
  L_print(F("[INFO]: ("));
  EH_printinfo(file, line);
  L_print(EH_GEN_END_MSG);
}

void EH_log_err(char *file, unsigned int line){
  L_print(EH_GEN_ST_MSG);
  L_print(F("[ERR](")); 
  EH_printinfo(file, line);
  L_print(EH_GEN_END_MSG); 
  EH_printerrp();
  if(errprint) errprint = ERR_NONEW;
}

void clrerr(){
  errno = 0; 
  errprint = 0; 

}

void seterr(uint8_t error){
  errno = error;
  errprint = error;

}

void EH_test(){
  L_println("Doing dbg test");
}

