
#ifndef logging_h
#define logging_h
#include <SoftwareSerial.h>
#include <string.h>
#include <inttypes.h>
#include <Stream.h>

#define LOGV_DEBUG 50
#define LOGV_INFO 40
#define LOGV_ERROR 30
#define LOGV_SILENT 0

#ifdef DEBUG
  #ifndef LOGLEVEL
  #define LOGLEVEL LOGV_DEBUG
  #endif
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
