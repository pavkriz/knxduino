#include "bus_hal.h"

BusHal::BusHal()
{
}

void BusHal::resetFlags()
{
    captureChannelFlag = false;
    timeChannelFlag = false;
}

void BusHal::isrCallbackCapture(TIM_HandleTypeDef* ahtim)
{
    if (ahtim->Instance == _timer.handle.Instance) {
        captureChannelFlag = true;        
    }
}

void BusHal::isrCallbackUpdate(TIM_HandleTypeDef* ahtim)
{
    if (ahtim->Instance == _timer.handle.Instance) {
        timeChannelFlag = true;
    }
}

void BusHal::_Error_Handler(char* filename, int line)
{
    while(1) {}
}
