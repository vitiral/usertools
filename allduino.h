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

#define RUN_ONLY_EVERY(time) \
    static uint16_t start = millis(); \
    if(((uint16_t)millis()) - start < time) \
        return; \
    start = millis();

#endif
