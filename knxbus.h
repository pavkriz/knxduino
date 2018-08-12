#ifndef KNXBUS_H
#define KNXBUS_H

#include "stm32f3xx_hal.h"

class KnxBus {

private:
	KnxBus();
    void COMP4Init();
    void TIM3Init();

public:
    int rxTimerValue = 0;
    TIM_HandleTypeDef htim3;
    COMP_HandleTypeDef hcomp4;
    void begin();
    void isrCallback(TIM_HandleTypeDef* htim);
    static KnxBus* getInstance()
        {
            // The only instance
            // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
            static KnxBus instance;
            return &instance;
        }

};

extern KnxBus Knx;

#endif