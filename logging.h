/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Library to provide Logger class that can print on either Serial or SoftSerial
 * and has other uses.
 */
 
#ifndef logging_h
#define logging_h
#include <SoftwareSerial.h>
#include <Stream.h>
#include <string.h>

#define LOG_DEBUG 50
#define LOG_INFO 40
#define LOG_ERROR 30
#define LOG_SILENT 0

#ifdef DEBUG
  #ifndef LOGLEVEL
  #define LOGLEVEL LOG_DEBUG
  #endif
#endif

// Dynamic Logging, used in error handling
#ifdef LOG_DYNAMIC  // allows you to turn logging on and off dynamically 
#ifdef LOGLEVEL
#if LOGLEVEL >= LOG_ERROR
  #define LOG_IFLL(LL,code) if(LOG_loglevel >= LL){code} 
  extern uint8_t LOG_loglevel;
  #define set_loglevel(LL) LOG_loglevel = LL
#endif
#endif
#endif

#ifndef LOG_IFLL(LL, code)
  #define LOG_IFLL(LL, code) code
  #define set_loglevel(LL)
#endif

class Logging : public Stream
{
private:
  uint8_t _mode;
  SoftwareSerial *_soft;
  
public:
  Logging();
  ~Logging();
  uint8_t silent;   // silent = true prevents Logger from sending any data.
                    // NOTE: No data is even added to buffer!!!
  uint16_t wrote;    // sets to true if any data was written

  void config_std();
  void config_soft(SoftwareSerial *soft);
  int peek();
  
  void repeat(char *c, int times);
  void repeat(char c, int times);

  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
  
  
  using Print::write;
};

extern Logging Logger;

#endif
