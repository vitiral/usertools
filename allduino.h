#ifndef __allduino_h
#define __allduino_h

// General include for all files.
// has some basic macros and includes proper arduino libraries
//

#ifdef SPARK
 #include <application.h>
#elif (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#ifdef SPARK
 #define ANALOGUE_MAX 4095
 #define LED_PIN 7
#else
 #define ANALOGUE_MAX 1023
 #define LED_PIN 13
#endif

#define TIME_SINCE(time) ((uint16_t)(millis() - (time)))

#define HEARTBEAT() \
    static uint16_t _heartbeat = millis(); \
    static uint8_t _hb_led = 2; \
    if(TIME_SINCE(_heartbeat) > 1000){ \
        if(_hb_led == 2){ \
            pinMode(LED_PIN, OUTPUT); \
            _hb_led = 0; \
        } \
        digitalWrite(LED_PIN, _hb_led); \
        _hb_led = 1 - _hb_led; \
    }

#define RUN_ONLY_EVERY(time) \
    static uint16_t start = millis(); \
    if(((uint16_t)millis()) - start < time) \
        return; \
    start = millis();

#endif
