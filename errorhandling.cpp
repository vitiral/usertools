
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "errorhandling.h"

const char *EH_GEN_END_MSG = ")|";
//const char EH_GEN_ST_MSG[] = {0xA, 0xD,':', ':', 0};
const char EH_GEN_ST_MSG[] = ":: ";
char *EH_EMPTY_STR = "";

char *errmsg = EH_EMPTY_STR;
uint8_t derr = 0;
uint8_t errno = 0;
uint8_t loglevel = 50;

EH_Serial_class::EH_Serial_class()
{
  _mode = EH_STD_SERIAL;
  _soft = 0;
}

EH_Serial_class::~EH_Serial_class()
{
}

void EH_Serial_class::config_std(){
  _mode = EH_STD_SERIAL;
}

// I can't get this working
void EH_Serial_class::config_soft(SoftwareSerial *soft){
  _mode = EH_SOFT_SERIAL;
  _soft = soft;
}

int EH_Serial_class::peek(){
  assert_return(0, -1);
}

size_t EH_Serial_class::write(uint8_t byte){
  switch(_mode){
  case EH_STD_SERIAL:
    return Serial.write(byte);
  case EH_SOFT_SERIAL:
    return _soft->write(byte);
  default:
    return 0;
  }
  return 0;
}

int EH_Serial_class::read(){
  assert_return(0, -1);
}

int EH_Serial_class::available(){
  assert_return(0, -1);
}

void EH_Serial_class::flush(){
  switch(_mode){
  case EH_STD_SERIAL:
    return Serial.flush();
  case EH_SOFT_SERIAL:
    return _soft->flush();
  default:
    assert_return(0);
  }
  assert_return(0);
}

EH_Serial_class EH_Serial;

void EH_printinfo(char *file, unsigned int line)
{
  EH_Serial.print(derr); 
  EH_Serial.write(' '); 
  EH_Serial.print(file); 
  EH_Serial.write(':'); 
  EH_Serial.print(line);
}

void EH_printerrno(){
  EH_Serial.write('(');
  EH_Serial.print(errno);
  EH_Serial.write(':');
  
  switch(errno){
  case 0:
    EH_Serial.print(F("NoErr")); break;
  case 1:
    EH_Serial.print(F("BaseErr")); break;
  case 2:
    EH_Serial.print(F("TimeoutErr")); break;
  case 3:
    EH_Serial.print(F("SerialErr")); break;
  case 4:
    EH_Serial.print(F("SpiErr")); break;
  case 5:
    EH_Serial.print(F("I2cErr")); break;
  case 6:
    EH_Serial.print(F("ComErr")); break;
  case 7:
    EH_Serial.print(F("ConfigErr")); break;
  case 8:
    EH_Serial.print(F("PinErr")); break;
  case 9:
    EH_Serial.print(F("InputErr")); break;
  
  case 50:
    EH_Serial.print(F("TypeErr")); break;
  case 51:
    EH_Serial.print(F("ValueErr")); break;
  case 52:
    EH_Serial.print(F("AssertErr")); break;
  case 53:
    EH_Serial.print(F("TestFail")); break;
  case 54:
    EH_Serial.print(F("PtrErr")); break;
  case 55:
    EH_Serial.print(F("IndexErr")); break;
  case 56:
    EH_Serial.print(F("SizeErr")); break;
  case 57:
    EH_Serial.print(F("ThreadErr")); break;
  
  case 252:;
    EH_Serial.print(F("Cleared Error")); break;
  case 253:
    EH_Serial.print(F("NoNew")); break;
  case 254:
    EH_Serial.print(EH_EMPTY_STR); break;
  case 255:
  default:
    EH_Serial.print(F("UnknownErr:")); break;
  }
  EH_Serial.write(')');
  EH_Serial.print(errmsg);
}

void EH_start_debug(char *file, unsigned int line){
  EH_Serial.print(EH_GEN_ST_MSG);
  EH_Serial.print(F("[DBG]: (")); 
  EH_printinfo(file, line); 
  EH_Serial.print(EH_GEN_END_MSG);
}

void EH_start_info(char *file, unsigned int line){
  EH_Serial.print(EH_GEN_ST_MSG);
  EH_Serial.print(F("[INFO]: ("));
  EH_printinfo(file, line);
  EH_Serial.print(EH_GEN_END_MSG);
}

void EH_log_err(char *file, unsigned int line){
  EH_Serial.print(EH_GEN_ST_MSG);
  EH_Serial.print(F("[ERR](")); 
  EH_printinfo(file, line);
  EH_Serial.print(EH_GEN_END_MSG); 
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
  EH_Serial.println("Doing dbg test");
}

