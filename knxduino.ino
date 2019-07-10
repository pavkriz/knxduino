#include "Arduino.h"

#ifdef STM32F3xx
#include "stm32f3xx_hal_comp.h"
#define LED_PIN PA5
#elif STM32G0xx
#include "stm32g0xx_hal_comp.h"
#define LED_PIN PA15
HardwareSerial Serial(USART1);
#else
#error "Only STM32F3xx and STM32G0xx devices are supported"
#endif

#include "tmpinit.h"

#include "src/sblib/sblib_default_objects.h"

#define KNX_RX_TRESHOLD_DAC_PIN PA4 // =DAC OUT 1 (DAC OUT 1 may be routed internally-only to COMParator, but for debugging purpose, it's nice to see the value on a pin)

int counter;

/*
 * Global HAL IRQ Callbacks.
 * May lead to duplicate definition in case it will
 * be implemented in STM32duino Core in future.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    // TODO filter particular timer ?
    busHal.isrCallbackCapture(htim);
    bus.timerInterruptHandler();  
}

static void isrArduinoTimerUpdateCallback(stimer_t *obj)
{
    // TODO filter particular timer ?
    busHal.isrCallbackUpdate(&obj->handle);
    bus.timerInterruptHandler();
}

void setup() {
   pinMode(LED_PIN, OUTPUT);
   Serial.begin(115200);
   //analogWrite(KNX_RX_TRESHOLD_DAC_PIN, 1.3*255/3.3);
  // // fix to connect DAC to PIN (arduino-way) + on-chip peripherals (COMP input)
  // PinName pin = digitalPinToPinName(KNX_RX_TRESHOLD_DAC_PIN);
  // DAC_HandleTypeDef DacHandle = {};
  // DAC_ChannelConfTypeDef dacChannelConf = {};
  // uint32_t dacChannel;
  // DacHandle.Instance = DAC1; // pinmap_peripheral(pin, PinMap_DAC); // TODO error: invalid conversion from 'void*' to 'DAC_TypeDef*' [-fpermissive]
  // if (DacHandle.Instance == NP) {
  //   Error_Handler();
  // }
  // dacChannel = DAC_CHANNEL_1; // get_dac_channel(pin); // TODO private mapping func in analog.c
  // if (!IS_DAC_CHANNEL(dacChannel)) {
  //   Error_Handler();
  // }
  // dacChannelConf.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  // dacChannelConf.DAC_Trigger = DAC_TRIGGER_NONE;
  // dacChannelConf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  // dacChannelConf.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  // /*##-2- Configure DAC channel1 #############################################*/
  // if (HAL_DAC_ConfigChannel(&DacHandle, &dacChannelConf, dacChannel) != HAL_OK) {
  //   /* Channel configuration Error */
  //   Error_Handler();
  // }

  // Serial.println("Start");  

  delay(1000);
  
  // Arduino interfacing for UPDATE event callback that is already handled by Arduino STM32 core,
  // beware, may interfere with libraries using STM32 hardware timers (PWM, Servo, SoftSerial,...)
  attachIntHandle(&busHal._timer, isrArduinoTimerUpdateCallback);

  bcu.begin(2, 1, 1); // ABB, dummy something device
  bcu.setOwnAddress(0xFFC0); // TODO EEPROM not implemented yet

  MX_DAC1_Init();
  MX_COMP1_Init();

  Serial.print("CLK: ");
  Serial.println(HAL_RCC_GetHCLKFreq());

  // Disable telegram processing by the lib
  if (userRam.status & BCU_STATUS_TL)
      userRam.status ^= BCU_STATUS_TL | BCU_STATUS_PARITY;
}

void loop() {
  //digitalWrite(PA5, HIGH);
  //delay(100);
  //digitalWrite(PA5, LOW);
  //delay(100);    
  
  //Serial.print("===  ");
  Serial.println(counter++);
  //Serial.println(HAL_COMP_GetState(&KnxBus::getInstance()->hcomp4));
  //Serial.println(HAL_COMP_GetOutputLevel(&KnxBus::getInstance()->hcomp4));
  //Serial.println(KnxBus::getInstance()->htim3.Instance->CNT);
  //Serial.println(busHal._timer.handle.Instance->CNT);
  //Serial.println(tim2handle.Instance->CCR4);
  
  //delay(1000);

  if (bus.telegramReceived()) {
        Serial.print("Telegram: ");
        for (int i = 0; i < bus.telegramLen; ++i) {
            if (i) Serial.print(" ");
            Serial.print(bus.telegram[i], HEX);
        }
        Serial.println();

        bus.discardReceivedTelegram();

    }

  unsigned char sendTelBuffer[32];
  sendTelBuffer[0] = 188;
  sendTelBuffer[1] = 17;
  sendTelBuffer[2] = 10;
  sendTelBuffer[3] = 0;
  sendTelBuffer[4] = 1;
  sendTelBuffer[5] = 225;
  sendTelBuffer[6] = 0;
  sendTelBuffer[7] = 128;
  bus.sendTelegram(sendTelBuffer, 8);
  delay(1000);

  sendTelBuffer[0] = 188;
  sendTelBuffer[1] = 17;
  sendTelBuffer[2] = 10;
  sendTelBuffer[3] = 0;
  sendTelBuffer[4] = 1;
  sendTelBuffer[5] = 225;
  sendTelBuffer[6] = 0;
  sendTelBuffer[7] = 129;
  bus.sendTelegram(sendTelBuffer, 8);
  delay(1000);

}
