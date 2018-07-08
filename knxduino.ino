#include "Arduino.h"

#ifndef STM32F3xx
#error "Only STM32F3xx devices are supported"
#endif

void setup() {
  pinMode(PA5, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(PA5, HIGH);
  delay(100);
  digitalWrite(PA5, LOW);
  delay(100);    
  Serial.println("Hi there!");
}
