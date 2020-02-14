/*
 *  BootLoader.c - The bootloader.
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "stm32g0xx.h"
#include "stm32g0xx_hal.h"
//#include "stm32g0xx_hal_pwr_ex.h"

#include <eib.h>
#include <timeout.h>
#include <internal/variables.h>
//#include <sblib/io_pin_names.h>
#include "bcu_updater.h"
#include "serial.h"

static BcuUpdate _bcu = BcuUpdate();
BcuBase& bcu = _bcu;

// The EIB bus access objects
#include <knxduino.h>
BusHal knxBusHal;
Bus knxBus(knxBusHal);

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
void TIM15_IRQHandler(void);
void SysTick_Handler(void);
#ifdef __cplusplus
 }
#endif

int tickNum = 0;

void TIM15_IRQHandler(void)
{
	TIM_HandleTypeDef *htim = &(knxBusHal._timer.handle);
	HAL_TIM_IRQHandler(htim);
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    // TODO filter particular timer ?
	knxBusHal.isrCallbackCapture(htim);
    knxBus.timerInterruptHandler();
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM15) {
		knxBusHal.isrCallbackUpdate(htim);
		knxBus.timerInterruptHandler();
	}
}


/**
 * Setup the library.
 */
static inline void lib_setup()
{
}

void setup()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

    // PIN_RUN: Configure PA15 as push-pull output (LED)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //pinMode(PIN_INFO, OUTPUT); // TODO


    bcu.begin(4, 0x2060, 1, KNXDUINO_ONE_PINMAPPING); // We are a "Jung 2138.10" device, version 0.1

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
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);	// PIN_RUN
        UART_printf("$ %02X\n", tickNum++);
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

void SystemClock_Config(void);

int main(void)
{
	//Initialize the HAL
	HAL_Init();
	SystemClock_Config();

	// disable buffering - worse performance, no Imprecise bus faults
	uint32_t *ACTLR = (uint32_t *)0xE000E008; *ACTLR = *ACTLR | 2;

    unsigned int * magicWord = (unsigned int *) APP_TO_BOOTLOADER_FLAG_ADDR;
    if (*magicWord == 0x5E1FB055)
    {
        *magicWord = 0;
        run_updater();
    }
    *magicWord = 0;

    // PIN_PROG: Configure PC15 as input (BUTTON), has external pullup
    __HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    if (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15))
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

void Error_Handler() {
	while (1) {}
}

// copied from Arduino STM32 core
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /* Configure the main internal regulator output voltage */
  //HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /* Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}
