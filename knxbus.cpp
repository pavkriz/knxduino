#include "knxbus.h"

#include "stm32_def.h"
#include "stm32f3xx_hal.h"

/*
 * Global HAL IRQ Callback.
 * May lead to duplicate definition in case it will
 * be implemented in STM32duino Core in future.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    (KnxBus::getInstance())->isrCallback(htim);
}

KnxBus::KnxBus()
{
}

void KnxBus::begin(void)
{
    COMP4Init();
    TIM3Init();
}

void KnxBus::COMP4Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;    

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

    hcomp4.Instance = COMP4;
    hcomp4.Init.InvertingInput = COMP_INVERTINGINPUT_DAC1_CH1;
    hcomp4.Init.NonInvertingInput = COMP_NONINVERTINGINPUT_IO1;
    hcomp4.Init.Output = COMP_OUTPUT_NONE;
    hcomp4.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp4.Init.BlankingSrce = COMP_BLANKINGSRCE_NONE;
    hcomp4.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
    if (HAL_COMP_Init(&hcomp4) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    HAL_COMP_Start(&hcomp4);
}

void KnxBus::TIM3Init()
{
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;   

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 72; // 72MHz/72 = 1MHz (ie. 1us tick)
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 4294967295; // 2^32-1
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0x0000;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    TIM_IC_InitTypeDef sConfigIC;

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 4;
    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    //sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    //sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    //sConfigIC.ICFilter = 0;
    //if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_4) != HAL_OK) {
    //    _Error_Handler(__FILE__, __LINE__);
    //}

    if (HAL_TIM_Base_Start(&htim3) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    //HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
}

void KnxBus::isrCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM3) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            rxTimerValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        }
    }
}