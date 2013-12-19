#include <SoftwareSerial.h>
#include <pt.h>

void setup(){
  Serial.begin(57600);
  pthread pt;
  pt.put_input((uint8_t)142);
  pt.put_input((int16_t)-42);
  pt.put_input("hello");
  pt.put_input(-32000);
  //pt.put_input((int32_t)0x7000000);
  //
  
  Serial.println("Data put");
  Serial.println(pt.get_int_input(0));
  Serial.println(pt.get_int_input(1));
  Serial.println(pt.get_str_input(2));
  Serial.println(pt.get_int_input(3));
  //Serial.println(pt.get_int_input(3), HEX);
  //Serial.println(0x7000000, HEX);
  //
}

void loop(){}

