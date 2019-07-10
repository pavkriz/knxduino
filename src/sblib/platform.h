#ifndef sblib_platform_h
#define sblib_platform_h

#ifdef STM32F3xx
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_flash_ex.h"
#elif STM32G0xx
#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_flash_ex.h"
#else
#error "Only STM32F3xx and STM32G0xx devices are supported"
#endif

#endif