
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "errorhandling.h"
#include "logging.h"
#include "MemoryFree.h"

const char *EH_GEN_END_MSG = ")|";
//const char EH_GEN_ST_MSG[] = {0xA, 0xD,':', ':', 0};
const char EH_GEN_ST_MSG[] = ":: ";
char *EH_EMPTY_STR = "";

char *errmsg = EH_EMPTY_STR;
uint8_t derr = 0;
uint8_t errno = 0;
uint8_t loglevel = 50;
uint8_t EH_flush = true;

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
  Logger.print(derr); 
  Logger.write(' '); 
  Logger.print(file); 
  Logger.write(':'); 
  Logger.print(line);
}

void EH_printerrno(){
  Logger.write('(');
  Logger.print(errno);
  Logger.write(':');
  
  switch(errno){
  case 0:
    Logger.print(F("NoErr")); break;
  case 1:
    Logger.print(F("BaseErr")); break;
  case 2:
    Logger.print(F("TimeoutErr")); break;
  case 3:
    Logger.print(F("SerialErr")); break;
  case 4:
    Logger.print(F("SpiErr")); break;
  case 5:
    Logger.print(F("I2cErr")); break;
  case 6:
    Logger.print(F("ComErr")); break;
  case 7:
    Logger.print(F("ConfigErr")); break;
  case 8:
    Logger.print(F("PinErr")); break;
  case 9:
    Logger.print(F("InputErr")); break;
  
  case 50:
    Logger.print(F("TypeErr")); break;
  case 51:
    Logger.print(F("ValueErr")); break;
  case 52:
    Logger.print(F("AssertErr")); break;
  case 53:
    Logger.print(F("TestFail")); break;
  case 54:
    Logger.print(F("PtrErr")); break;
  case 55:
    Logger.print(F("IndexErr")); break;
  case 56:
    Logger.print(F("SizeErr")); break;
  case 57:
    Logger.print(F("ThreadErr")); break;
  
  case 252:;
    Logger.print(F("Cleared Error")); break;
  case 253:
    Logger.print(F("NoNew")); break;
  case 254:
    Logger.print(EH_EMPTY_STR); break;
  case 255:
  default:
    Logger.print(F("UnknownErr:")); break;
  }
  Logger.write(')');
  Logger.print(errmsg);
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
  EH_printerrno();
  if(errno) errno = ERR_NONEW;
  errmsg = EH_EMPTY_STR;
}

void clrerr(){
  derr = 0; 
  errno = 0; 
  errmsg = EH_EMPTY_STR;
}

void seterr(uint8_t error){
  derr = error;
  errno = error;
  errmsg = EH_EMPTY_STR;
}

void EH_test(){
  Logger.println("Doing dbg test");
}

