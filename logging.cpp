
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "logging.h"
#include "errorhandling.h"

Logging::Logging()
{
  _mode = EH_STD_SERIAL;
  _soft = 0;
}

Logging::~Logging()
{
}

void Logging::config_std(){
  _mode = EH_STD_SERIAL;
}

// I can't get this working
void Logging::config_soft(SoftwareSerial *soft){
  _mode = EH_SOFT_SERIAL;
  _soft = soft;
}

int Logging::peek(){
  assert_return(0, -1);
}

size_t Logging::write(uint8_t byte){
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

int Logging::read(){
  assert_return(0, -1);
}

int Logging::available(){
  assert_return(0, -1);
}

void Logging::flush(){
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

Logging Logger;
