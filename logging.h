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

#ifndef LOGLEVEL
#define LOGLEVEL 0
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

//#ifdef SoftwareSerial_h
#if LOGLEVEL > 0

#define LOG_STD_SERIAL 0
#define LOG_SOFT_SERIAL 1

#include <SoftwareSerial.h>
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

#define L_set_silent(S)       Logger.silent = S
#define L_silent              Logger.silent
#define L_set_wrote(w)        Logger.wrote = w
#define L_wrote               Logger.wrote
#define L_config_std()        Logger.config_std()
#define L_config_soft(...)    Logger.config_soft(__VA_ARGS__)
#define L_peek()              Logger.peek()
#define L_repeat(...)         Logger.repeat(__VA_ARGS__)
#define L_write(c)            Logger.write(c)
#define L_print(...)          Logger.print(__VA_ARGS__)
#define L_println(...)        Logger.println(__VA_ARGS__)
#define L_read()              Logger.read()
#define L_available()         Logger.available()
#define L_flush()             Logger.flush()

extern Logging Logger;

#else

#define L_set_silent(S) 
#define L_silent              true
#define L_set_wrote(w)        
#define L_wrote               false
#define L_config_std()    
#define L_config_soft(...) 
#define L_peek()              0
#define L_repeat(...)
#define L_write(c)            0            
#define L_print(...)          0  
#define L_println(...)        0
#define L_read()              0
#define L_available()         false
#define L_flush()

#endif

#endif
