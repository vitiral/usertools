
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "logging.h"

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
