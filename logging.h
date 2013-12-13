
#ifndef logging_h
#define logging_h
#include <SoftwareSerial.h>
#include <string.h>
#include <inttypes.h>
#include <Stream.h>

#define LOG_DEBUG 50
#define LOG_INFO 40
#define LOG_ERROR 30
#define LOG_SILENT 0

#ifdef DEBUG
  #ifndef LOGLEVEL
  #define LOGLEVEL LOG_DEBUG
  #endif
#endif

#ifdef LOGLEVEL
  #if LOGLEVEL >= LOG_ERROR
    extern uint8_t LOG_loglevel;
    #define set_loglevel(LL) LOG_loglevel = LL
    // used in errorhandling
    #define LOG_IFLL(LL,code) if(LOG_loglevel >= LL){code} 
  #else
    #define set_loglevel(LL)
  #endif
#else
  #define set_loglevel(LL)
  #define LOGLEVEL LOG_SILENT
#endif


class Logging : public Stream
{
private:
  uint8_t _mode;
  SoftwareSerial *_soft;
 
public:
  Logging();
  //Logging(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
  ~Logging();
//  void begin(long speed);
//  bool listen();
//  void end();
//  bool isListening() { return this == active_object; }
//  bool overflow() { bool ret = _buffer_overflow; _buffer_overflow = false; return ret; }
//  bool overflow();

  void config_std();
  void config_soft(SoftwareSerial *soft);
  int peek();
  virtual size_t write(uint8_t byte);
  virtual int read();
  virtual int available();
  virtual void flush();
  
  using Print::write;
};

extern Logging Logger;

#endif
