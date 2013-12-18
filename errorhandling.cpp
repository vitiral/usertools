/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Library to help with debugging. See user guide in documentation.
 */
 
#include <Arduino.h>
#include <SoftwareSerial.h>
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
  Logger.print("Mem:");
  Logger.print(time);
  Logger.print("us FM:");
  Logger.print(mem);
#else
  Logger.print(F("M:"));
  Logger.print(freeMemory());
#endif
}
  
void EH_printinfo(char *file, unsigned int line)
{
  Logger.print(errno); 
  Logger.write(' '); 
  Logger.print(file); 
  Logger.write(':'); 
  Logger.print(line);
}

void EH_printerrp(){
  Logger.write('(');
  Logger.print(errprint);
  Logger.write(':');
  
  switch(errprint){
  case ERR_NOERR:
    Logger.print(F("No")); break;
  case ERR_BASE:
    Logger.print(F("Base")); break;
  case ERR_TIMEOUT:
    Logger.print(F("Timeout")); break;
  case ERR_SERIAL:
    Logger.print(F("Serial")); break;
  case ERR_SPI:
    Logger.print(F("Spi")); break;
  case ERR_I2C:
    Logger.print(F("I2c")); break;
  case ERR_COM:
    Logger.print(F("Com")); break;
  case ERR_CONFIG:
    Logger.print(F("Config")); break;
  case ERR_PIN:
    Logger.print(F("Pin")); break;
  case ERR_INPUT:
    Logger.print(F("Input")); break;
  
  case ERR_TYPE:
    Logger.print(F("Type")); break;
  case ERR_VALUE:
    Logger.print(F("Value")); break;
  case ERR_ASSERT:
    Logger.print(F("Assert")); break;
  case ERR_TESTFAIL:
    Logger.print(F("TestFail")); break;
  case ERR_PTR:
    Logger.print(F("Ptr")); break;
  case ERR_INDEX:
    Logger.print(F("Index")); break;
  case ERR_SIZE:
    Logger.print(F("Size")); break;
  case ERR_THREAD:
    Logger.print(F("Thread")); break;
  case ERR_MEMORY:
    Logger.print(F("Mem")); break;
  
  case ERR_CLEARED:
    Logger.print(F("Cleared or")); break;
  case ERR_NONEW:
    Logger.print(F("NoNew")); break;
  case ERR_EMPTY:
    Logger.print(EH_EMPTY_STR); break;
  case ERR_UNKNOWN:
  default:
    Logger.print(F("Unknown")); break;
  }
  Logger.print(F("Err"));
  Logger.write(')');
}

void EH_start_debug(char *file, unsigned int line){
  Logger.print(EH_GEN_ST_MSG);
  Logger.print(F("[DBG]:"));
  printmem();
  Logger.print(F("(")); 
  EH_printinfo(file, line); 
  Logger.print(EH_GEN_END_MSG);
}

void EH_start_info(char *file, unsigned int line){
  Logger.print(EH_GEN_ST_MSG);
  Logger.print(F("[INFO]: ("));
  EH_printinfo(file, line);
  Logger.print(EH_GEN_END_MSG);
}

void EH_log_err(char *file, unsigned int line){
  Logger.print(EH_GEN_ST_MSG);
  Logger.print(F("[ERR](")); 
  EH_printinfo(file, line);
  Logger.print(EH_GEN_END_MSG); 
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
  Logger.println("Doing dbg test");
}

