/*
 *  BootLoader.c - The bootloader.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"

#include <sblib/eib.h>
#include <sblib/timeout.h>
#include <sblib/internal/variables.h>
//#include <sblib/io_pin_names.h>
#include "bcu_updater.h"
#include "serial.h"

static BcuUpdate _bcu = BcuUpdate();
BcuBase& bcu = _bcu;

// The EIB bus access objects
BusHal busHal;
Bus bus(busHal);

#include <boot_descriptor_block.h>
//#include "sblib/digital_pin.h"
//#include "sblib/io_pin_names.h"
//#include <sblib/serial.h>

Timeout blinky;

/*
 * Global HAL IRQ Callbacks.
 */

#ifdef __cplusplus
 extern "C" {
#endif
void TIM3_IRQHandler(void);
#ifdef __cplusplus
 }
#endif

void TIM3_IRQHandler(void)
{
	TIM_HandleTypeDef *htim = &(busHal._timer.handle);
	HAL_TIM_IRQHandler(htim);
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    // TODO filter particular timer ?
    busHal.isrCallbackCapture(htim);
    bus.timerInterruptHandler();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	} else {
	   // TODO filter particular timer ?
		busHal.isrCallbackUpdate(htim);
		bus.timerInterruptHandler();
	}
}


void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  UNUSED(hdac);

  __HAL_RCC_DAC1_CLK_ENABLE();

  // optional PIN configuration, DAC_CH1 is internally connected to comparator
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


/**
 * Setup the library.
 */
static inline void lib_setup()
{
	//Initialize the HAL
	HAL_Init();
}

void setup()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

    // PIN_RUN: Configure PA5 as push-pull output (LED)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //pinMode(PIN_INFO, OUTPUT); // TODO

    // Setup DAC output
    //analogWrite(KNX_RX_TRESHOLD_DAC_PIN, 1.4*255/3.3);

    DAC_HandleTypeDef hdac;
    DAC_ChannelConfTypeDef sConfig;

    hdac.Instance = DAC1;
    HAL_DAC_Init(&hdac);

    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);

    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (int)(1.4*255/3.3) << 4);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

    bcu.begin(4, 0x2060, 1); // We are a "Jung 2138.10" device, version 0.1

    blinky.start(1);
    bcu.setOwnAddress(0xFFC0);
    extern byte userEepromModified;
    userEepromModified = 0;
}

void loop()
{
    if (blinky.expired())
    {
        if (bcu.directConnection())
            blinky.start(250);
        else
            blinky.start(1000);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);	// PIN_RUN
    }
    //digitalWrite(PIN_PROG, digitalRead(PIN_RUN)); // TODO
}

void jumpToApplication(unsigned int start)
{
    unsigned int StackTop = *(unsigned int *) (start);
    unsigned int ResetVector = *(unsigned int *) (start + 4);

    // relocate Vector Table to application's Vector Table (start must be 256B-aligned!)

    SCB->VTOR = start;

    // TODO disable and clear all interrupts here?

    /* Normally during RESET the stack pointer will be loaded
     * with the value stored location 0x0. Since the vector
     * table of the application is not located at 0x0 we have to do this
     * manually to ensure a correct stack.
     */
    asm volatile ("mov SP, %0" : : "r" (StackTop));
    /* Once the stack is setup we jump to the application reset vector */
    asm volatile ("bx      %0" : : "r" (ResetVector));
}

void run_updater()
{
    lib_setup();
#ifdef DUMP_TELEGRAMS
    serial_setup();
#endif
    setup();

    while (1)
    {
        bcu.loop();
        loop();
    }
}

int main(void)
{
	// disable buffering - worse performance, no Imprecise bus faults
	uint32_t *ACTLR = (uint32_t *)0xE000E008; *ACTLR = *ACTLR | 2;

    unsigned int * magicWord = (unsigned int *) 0x10000000;
    if (*magicWord == 0x5E1FB055)
    {
        *magicWord = 0;
        run_updater();
    }
    *magicWord = 0;

    // PIN_PROG: Configure PC13 as input (BUTTON), has external pullup
    __HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
    {
        run_updater();
    }

    // TODO wait a second or two for a magic telegram before jumping to the application

    AppDescriptionBlock * block = (AppDescriptionBlock *) FIRST_SECTOR;
    block--;
    for (int i = 0; i < 2; i++, block--)
    {
        if (checkApplication(block))
            jumpToApplication(block->startAddress);
    }
    run_updater();
    return 0;
}
