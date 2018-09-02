/*
 *  bus_hal.h - Hardware abstraction layer for Low level EIB bus access.
 *              It encapsulates timer and GPIO-related operations.
 *              Intended to be re-implemented for different platforms.
 *              In general, it requires a 16-bit timer with capture input (+interrupt), 
 *              match to output (PWM), and match to interrupt capabilities.
 
 *              We use the preprocessor here in order to avoid virtual methods 
 *              (and their performance penalty). May be a subject to reconsider.
 *              Probably make the Bus class a template in order to hint the compiler
 *              which BusHal implementation is used.
 *              
 *
 *  Copyright (c) 2018 Pavel Kriz <pavkriz@hkfree.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bus_hal_h
#define sblib_bus_hal_h

#ifdef STM32F303xE

#include "stm32f3xx_hal.h"
#include <timer.h>
#include <Arduino.h>

/*
 *    STM32 implementation
 */

class BusHal
{
public:
    BusHal();
    void begin();
    void idleState();
    void waitBeforeSending(unsigned int timeValue);
    bool isCaptureChannelFlag();
    bool isTimeChannelFlag();
    void setupTimeChannel(unsigned int value);
    unsigned int getCaptureValue();
    void setTimeMatch(unsigned int value);
    void setupCaptureWithInterrupt();
    void setupCaptureWithoutInterrupt();
    void resetFlags();
    void disableInterrupts();
    void enableInterrupts();
    void startSending();
    void setupStartBit(int pwmMatch, int timeMatch);
    unsigned int getTimerValue();
    unsigned int getPwmMatch();
    void setPwmMatch(unsigned int pwmMatch);
    void isrCallbackCapture(TIM_HandleTypeDef* ahtim);
    void isrCallbackUpdate(TIM_HandleTypeDef* ahtim);
    void _Error_Handler(char* filename, int line);
    stimer_t _timer; // TODO static?
protected:
    int rxTimerValue = 0;
    COMP_HandleTypeDef hcomp;
    bool captureChannelFlag = false;
    bool timeChannelFlag = false;

};

inline void BusHal::begin()
{
    // COMP init
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

    hcomp.Instance = COMP4;
    hcomp.Init.InvertingInput = COMP_INVERTINGINPUT_DAC1_CH1;
    hcomp.Init.NonInvertingInput = COMP_NONINVERTINGINPUT_IO1;
    hcomp.Init.Output = COMP_OUTPUT_NONE;
    hcomp.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp.Init.BlankingSrce = COMP_BLANKINGSRCE_NONE;
    hcomp.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
    if (HAL_COMP_Init(&hcomp) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    HAL_COMP_Start(&hcomp);

    // TIM init

    // Arduino interfacing for UPDATE event callback that is already handled by Arduino STM32 core,
    // beware, may interfere with libraries using STM32 hardware timers (PWM, Servo, SoftSerial,...)
    _timer.timer = TIM3;
    // mimics TimerHandleInit:
    TIM_HandleTypeDef *htim = &(_timer.handle);

    // HAL stuff
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    htim->Instance = _timer.timer;
    htim->Init.Prescaler = 72; // 72MHz/72 = 1MHz (ie. 1us tick)
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 65535; // 2^16-1
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.RepetitionCounter = 0x0000;
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    TIM_IC_InitTypeDef sConfigIC;
    TIM_OC_InitTypeDef sConfigOC;

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 4;
    if (HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0xffff;    // no pulses, all high (means no pulses)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_2);
    // the stupid HAL_TIM_PWM_ConfigChannel automaticaly enables TIM_CCMR1_OC2PE preload bit, but we dont want to use it
    htim->Instance->CCMR1 &= ~TIM_CCMR1_OC2PE;  // disable TIM_CCMR1_OC2PE

    if (HAL_TIM_Base_Start(htim) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
    // TODO? make sure we don't make any unintentional pulse here
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
}

inline void BusHal::idleState()
{
    _timer.handle.Instance->ARR = 0xfffe;
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_CC1);
    __HAL_TIM_DISABLE_IT(&_timer.handle, TIM_IT_UPDATE);
    _timer.handle.Instance->CCR2 = 0xffff;   // no pulses
}

inline void BusHal::waitBeforeSending(unsigned int timeValue) 
{
    _timer.handle.Instance->ARR = (uint32_t)timeValue;
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_UPDATE); // clear UPDATE interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_UPDATE); // enable UPDATE interrupt
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_CC1); // clear Capture Channel 1 interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_CC1); // enable Channel 1 interrupt flag here
}

inline bool BusHal::isCaptureChannelFlag()
{
    return captureChannelFlag;
}

inline bool BusHal::isTimeChannelFlag()
{
    return timeChannelFlag;
}

inline void BusHal::setupTimeChannel(unsigned int value)
{
    _timer.handle.Instance->ARR = (uint32_t)value; // new period
    _timer.handle.Instance->CNT = 0; // reset counter
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_UPDATE); // clear UPDATE interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_UPDATE); // enable UPDATE interrupt
}

inline unsigned int BusHal::getCaptureValue()
{
    return _timer.handle.Instance->CCR1;
}

inline void BusHal::setupStartBit(int pwmMatch, int timeMatch) 
{
    _timer.handle.Instance->CCR2 = (uint32_t)pwmMatch;
    _timer.handle.Instance->ARR = (uint32_t)timeMatch; // new period
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_UPDATE); // clear UPDATE interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_UPDATE); // enable UPDATE interrupt
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_CC1); // clear Capture Channel 1 interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_CC1); // enable Channel 1 interrupt flag here
}

inline unsigned int BusHal::getTimerValue()
{
    return _timer.handle.Instance->CNT;
}

inline unsigned int BusHal::getPwmMatch()
{
    return _timer.handle.Instance->CCR2;
}

inline void BusHal::setPwmMatch(unsigned int pwmMatch)
{
    _timer.handle.Instance->CCR2 = (uint32_t)pwmMatch;
}

inline void BusHal::setTimeMatch(unsigned int timeMatch)
{
    _timer.handle.Instance->ARR = (uint32_t)timeMatch; // new period
}

inline void BusHal::setupCaptureWithInterrupt()
{
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_CC1); // clear Capture Channel 1 interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_CC1); // enable Channel 1 interrupt flag here
}

inline void BusHal::setupCaptureWithoutInterrupt()
{
    __HAL_TIM_DISABLE_IT(&_timer.handle, TIM_IT_CC1); // disable Channel 1 interrupt flag here
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_CC1); // clear Capture Channel 1 interrupt flag here
}

inline void BusHal::disableInterrupts()
{
    __disable_irq();
}

inline void BusHal::enableInterrupts()
{
    __enable_irq();
}

inline void BusHal::startSending()
{
    _timer.handle.Instance->ARR = 1;
    __HAL_TIM_CLEAR_FLAG(&_timer.handle, TIM_FLAG_UPDATE); // clear UPDATE interrupt flag here
    __HAL_TIM_ENABLE_IT(&_timer.handle, TIM_IT_UPDATE); // enable UPDATE interrupt
    _timer.handle.Instance->CNT = 0; // reset counter
}

#else

// Mask for the timer flag of the capture channel
#define CAPTURE_FLAG (8 << captureChannel)

// Mask for the timer flag of the time channel
#define TIME_FLAG (8 << timeChannel)

/*
 *    original LPC implementation
 */

class BusHal
{
public:
    BusHal(Timer& timer, int rxPin, int txPin, TimerCapture captureChannel, TimerMatch matchChannel);
    void begin();
    void idleState();
    void waitBeforeSending();
    bool isCaptureChannelFlag();
    bool isTimeChannelFlag();
    void setupTimeChannel(unsigned int value);
    unsigned int getCaptureValue();
    void setTimeMatch(unsigned int value);
    void setupCaptureWithInterrupt();
    void setupCaptureWithoutInterrupt();
    void resetFlags();
    void disableInterrupts();
    void enableInterrupts();
    void startSending();
    void setupStartBit(int pwmMatch, int timeMatch);
    unsigned int getTimerValue();
    unsigned int getPwmMatch();
    void setPwmMatch(unsigned int pwmMatch);
protected:
    Timer& timer;                //!< The timer
    int rxPin, txPin;            //!< The pins for bus receiving and sending
    TimerCapture captureChannel; //!< The timer channel that captures the timer value on the bus-in pin
    TimerMatch pwmChannel;       //!< The timer channel for PWM for sending
    TimerMatch timeChannel;      //!< The timer channel for timeouts

};

BusHal::BusHal(Timer& aTimer, int aRxPin, int aTxPin, TimerCapture aCaptureChannel, TimerMatch aPwmChannel)
:timer(aTimer)
,rxPin(aRxPin)
,txPin(aTxPin)
,captureChannel(aCaptureChannel)
,pwmChannel(aPwmChannel)
{
    timeChannel = (TimerMatch) ((pwmChannel + 2) & 3);  // +2 to be compatible to old code during refactoring
}

inline void BusHal::begin()
{
    timer.begin();
    timer.pwmEnable(pwmChannel);
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
    timer.start();
    timer.interrupts();
    timer.prescaler(TIMER_PRESCALER);

    timer.match(timeChannel, 0xfffe);
    timer.matchMode(timeChannel, RESET);
    timer.match(pwmChannel, 0xffff);

    // wait until output is driven low before enabling output pin.
    // Using digitalWrite(txPin, 0) does not work with MAT channels.
    timer.value(0xffff); // trigger the next event immediately
    while (timer.getMatchChannelLevel(pwmChannel) == true);
    pinMode(txPin, OUTPUT_MATCH);   // Configure bus output
    pinMode(rxPin, INPUT_CAPTURE | HYSTERESIS);  // Configure bus input
}

inline void BusHal::idleState()
{
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);

    timer.matchMode(timeChannel, RESET);
    timer.match(timeChannel, 0xfffe);
    timer.match(pwmChannel, 0xffff);
}

inline void BusHal::waitBeforeSending() 
{
    timer.match(timeChannel, sendAck ? SEND_ACK_WAIT_TIME - PRE_SEND_TIME : SEND_WAIT_TIME - PRE_SEND_TIME);
    timer.matchMode(timeChannel, INTERRUPT | RESET);

    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);

}

inline bool BusHal::isCaptureChannelFlag()
{
    return timer.flag(captureChannel);
}

inline bool BusHal::isTimeChannelFlag()
{
    return timer.flag(timeChannel);
}

inline void BusHal::setupTimeChannel(unsigned int value)
{
    timer.match(timeChannel, value);
    timer.restart();
    timer.matchMode(timeChannel, INTERRUPT | RESET);
}

inline unsigned int BusHal::getCaptureValue()
{
    return timer.capture(captureChannel);
}

inline void BusHal::setupStartBit(int pwmMatch, int timeMatch)
{
    timer.match(pwmChannel, pwmMatch);
    timer.match(timeChannel, timeMatch);
    timer.matchMode(timeChannel, RESET | INTERRUPT);
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
}

inline unsigned int BusHal::getTimerValue()
{
    return timer.value();
}

inline unsigned int BusHal::getPwmMatch()
{
    return timer.match(pwmChannel);
}

inline void BusHal::setPwmMatch(unsigned int pwmMatch)
{
    timer.match(pwmChannel, pwmMatch);
}

inline void BusHal::setTimeMatch(unsigned int timeMatch)
{
    timer.match(timeChannel, timeMatch);
}

inline void BusHal::setupCaptureWithInterrupt()
{
    timer.captureMode(captureChannel, FALLING_EDGE | INTERRUPT);
}

inline void BusHal::setupCaptureWithoutInterrupt()
{
    timer.captureMode(captureChannel, FALLING_EDGE);
}

inline void BusHal::resetFlags()
{
    timer.resetFlags();
}

inline void BusHal::disableInterrupts()
{
    noInterrupts();
}

inline void BusHal::enableInterrupts()
{
    interrupts();
}

inline void BusHal::startSending()
{
    timer.match(timeChannel, 1);
    timer.matchMode(timeChannel, INTERRUPT | RESET);
    timer.value(0);
}

#endif

#endif /*sblib_bus_hal_h*/
