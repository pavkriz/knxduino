#include "Arduino.h"

#include "stm32f3xx_hal_comp.h"

#ifndef STM32F3xx
#error "Only STM32F3xx devices are supported"
#endif

#include "knxbus.h"

#define LED_PIN PA5
#define KNX_RX_TRESHOLD_DAC_PIN PA4

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  analogWrite(KNX_RX_TRESHOLD_DAC_PIN, 1.4*255/3.3);
  Serial.println("Start");
  KnxBus::getInstance()->begin();
}

void loop() {
  //digitalWrite(PA5, HIGH);
  //delay(100);
  //digitalWrite(PA5, LOW);
  //delay(100);    
  
  Serial.println("===");
  //Serial.println(HAL_COMP_GetState(&KnxBus::getInstance()->hcomp4));
  //Serial.println(HAL_COMP_GetOutputLevel(&KnxBus::getInstance()->hcomp4));
  //Serial.println(KnxBus::getInstance()->htim3.Instance->CNT);
  //Serial.println(KnxBus::getInstance()->htim3.Instance->CCR1);
  //Serial.println(tim2handle.Instance->CCR4);
  Serial.println(KnxBus::getInstance()->rxTimerValue);
  delay(1000);
}
