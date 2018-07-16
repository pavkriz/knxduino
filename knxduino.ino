#include "Arduino.h"

#include "stm32f3xx_hal_comp.h"

#ifndef STM32F3xx
#error "Only STM32F3xx devices are supported"
#endif

#define LED_PIN PA5
#define KNX_RX_TRESHOLD_DAC_PIN PA4
#define KNX_RX_DIGITAL_INPUT PA7

COMP_HandleTypeDef hcomp4;

void HAL_COMP_MspInit(COMP_HandleTypeDef* hcomp)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hcomp->Instance==COMP4)
  {
    /**COMP4 GPIO Configuration    
    PB0     ------> COMP4_INP
    PB1     ------> COMP4_OUT 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_COMP4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }

}

static void MX_COMP4_Init(void)
{

  hcomp4.Instance = COMP4;
  hcomp4.Init.InvertingInput = COMP_INVERTINGINPUT_DAC1_CH1;
  hcomp4.Init.NonInvertingInput = COMP_NONINVERTINGINPUT_IO1;
  hcomp4.Init.Output = COMP_OUTPUT_NONE;
  hcomp4.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp4.Init.BlankingSrce = COMP_BLANKINGSRCE_NONE;
  hcomp4.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  if (HAL_COMP_Init(&hcomp4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

void setup() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  analogWrite(KNX_RX_TRESHOLD_DAC_PIN, 1.4*255/3.3);
  MX_COMP4_Init(); // KNX RX comparator
  HAL_COMP_Start(&hcomp4);
}

void loop() {
  //digitalWrite(PA5, HIGH);
  //delay(100);
  //digitalWrite(PA5, LOW);
  //delay(100);    
  Serial.println("===");
  Serial.println(HAL_COMP_GetState(&hcomp4));
  Serial.println(HAL_COMP_GetOutputLevel(&hcomp4));
}
