/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Library to provide Logger class that can print on either Serial or SoftSerial
 * and has other uses.
 */
 
#include <Arduino.h>
#include <SoftwareSerial.h>
#define LOGLEVEL 1

#include "logging.h"
#include "errorhandling.h"

uint8_t LOG_loglevel = LOG_DEBUG;

Logging::Logging()
{
  _mode = LOG_STD_SERIAL;
  _soft = 0;
  silent = false;
  wrote = 0;
}

Logging::~Logging()
{
}

void Logging::config_std(){
  _mode = LOG_STD_SERIAL;
}

// I can't get this working
void Logging::config_soft(SoftwareSerial *soft){
  _mode = LOG_SOFT_SERIAL;
  _soft = soft;
}

int Logging::peek(){
  switch(_mode){
  case LOG_STD_SERIAL:
    return Serial.peek();
  case LOG_SOFT_SERIAL:
    return _soft->peek();
  default:
    return 0;
  }
  return 0;
}

size_t Logging::write(uint8_t byte){
  if(silent) return 0;
  wrote += 1;
  switch(_mode){
  case LOG_STD_SERIAL:
    return Serial.write(byte);
  case LOG_SOFT_SERIAL:
    return _soft->write(byte);
  default:
    return 0;
  }
}

int Logging::read(){
  switch(_mode){
  case LOG_STD_SERIAL:
    return Serial.read();
  case LOG_SOFT_SERIAL:
    return _soft->read();
  default:
    return 0;
  }
  return 0;
}

int Logging::available(){
  switch(_mode){
  case LOG_STD_SERIAL:
    return Serial.available();
  case LOG_SOFT_SERIAL:
    return _soft->available();
  default:
    return 0;
  }
  return 0;
}

void Logging::flush(){
  switch(_mode){
  case LOG_STD_SERIAL:
    return Serial.flush();
  case LOG_SOFT_SERIAL:
    return _soft->flush();
  default:
    assert_return(0);
  }
  assert_return(0);
}

void Logging::repeat(char *c, int times){
  for(int t = 0; t < times; t ++){
    Logger.print(c);
  }
}

void Logging::repeat(char c, int times){
  for(int t = 0; t < times; t ++){
    Logger.write(c);
  }
}

Logging Logger;
