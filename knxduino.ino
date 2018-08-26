#include "Arduino.h"

#include "stm32f3xx_hal_comp.h"

#ifndef STM32F3xx
#error "Only STM32F3xx devices are supported"
#endif

#include "src/sblib/eib/bus_hal.h"
#include "src/sblib/eib/bus.h"

#define LED_PIN PA5
#define KNX_RX_TRESHOLD_DAC_PIN PA4

int counter;
BusHal busHal;
Bus bus(busHal);

/*
 * Global HAL IRQ Callbacks.
 * May lead to duplicate definition in case it will
 * be implemented in STM32duino Core in future.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    busHal.isrCallbackCapture(htim);
    bus.timerInterruptHandler();
}

static void isrArduinoTimerUpdateCallback(stimer_t *obj)
{
    busHal.isrCallbackUpdate(&obj->handle);
    bus.timerInterruptHandler();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  analogWrite(KNX_RX_TRESHOLD_DAC_PIN, 1.4*255/3.3);
  Serial.println("Start");
  
  // Arduino interfacing for UPDATE event callback that is already handled by Arduino STM32 core,
  // beware, may interfere with libraries using STM32 hardware timers (PWM, Servo, SoftSerial,...)
  attachIntHandle(&busHal._timer, isrArduinoTimerUpdateCallback);

  busHal.begin();
  bus.begin();
}

void loop() {
  //digitalWrite(PA5, HIGH);
  //delay(100);
  //digitalWrite(PA5, LOW);
  //delay(100);    
  
  Serial.print("===  ");
  Serial.println(counter++);
  //Serial.println(HAL_COMP_GetState(&KnxBus::getInstance()->hcomp4));
  //Serial.println(HAL_COMP_GetOutputLevel(&KnxBus::getInstance()->hcomp4));
  //Serial.println(KnxBus::getInstance()->htim3.Instance->CNT);
  Serial.println(busHal._timer.handle.Instance->CCR1);
  //Serial.println(tim2handle.Instance->CCR4);
  
  delay(1000);
}
