inline void BusHal::begin()
{
    // COMP init
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
    /**COMP1 GPIO Configuration    
    PA0     ------> COMP1_OUT
    PA1     ------> COMP1_INP 
    */

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_COMP1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // tmp TODO
    
    // // PB1     ------> COMP1_INM 
    // GPIO_InitStruct.Pin = GPIO_PIN_1;
    // GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hcomp.Instance = COMP1;
    hcomp.Init.InputPlus = COMP_INPUT_PLUS_IO3;
    //hcomp.Init.InputMinus = COMP_INPUT_MINUS_IO1; // TMP TODO
    hcomp.Init.InputMinus = COMP_INPUT_MINUS_DAC1_CH1;// TMP TODO
    hcomp.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp.Init.WindowOutput = COMP_WINDOWOUTPUT_EACH_COMP;
    hcomp.Init.Hysteresis = COMP_HYSTERESIS_HIGH;   // 30mV
    hcomp.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp.Init.Mode = COMP_POWERMODE_HIGHSPEED;
    hcomp.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
    hcomp.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
    if (HAL_COMP_Init(&hcomp) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_COMP_Start(&hcomp) != HAL_OK) {
      Error_Handler();
    }

    // tmp 

    // GPIO_InitStruct.Pin = GPIO_PIN_2;
    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // TIM init

    // Arduino interfacing for UPDATE event callback that is already handled by Arduino STM32 core,
    // beware, may interfere with libraries using STM32 hardware timers (PWM, Servo, SoftSerial,...)
    _timer.timer = TIM15;
    // mimics TimerHandleInit:
    TIM_HandleTypeDef *htim = &(_timer.handle);

    // HAL stuff
    __HAL_RCC_TIM15_CLK_ENABLE();
  
    /**TIM15 GPIO Configuration    
    PA2     ------> TIM15_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_TIM15;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**TIM15 GPIO Configuration    
    PA3     ------> TIM15_CH2 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_TIM15;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_EnableIRQ(TIM15_IRQn);

    htim->Instance = TIM15;
    htim->Init.Prescaler = 64; // 64MHz/64 = 1MHz (ie. 1us tick) TODO calc base on real CLK
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 65535; // 2^16-1
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.RepetitionCounter = 0;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    if (HAL_TIM_IC_Init(htim) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_TIM_PWM_Init(htim) != HAL_OK)
    {
      Error_Handler();
    }

    TIM_IC_InitTypeDef sConfigIC;
    TIM_OC_InitTypeDef sConfigOC;

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 4;
    if (HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
      Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
      Error_Handler();
    }
    // the stupid HAL_TIM_PWM_ConfigChannel automaticaly enables TIM_CCMR1_OC2PE preload bit, but we dont want to use it
    htim->Instance->CCMR1 &= ~TIM_CCMR1_OC2PE;  // disable TIM_CCMR1_OC2PE
    
    if (HAL_TIM_Base_Start(htim) != HAL_OK) {
      Error_Handler();
    }

    if (HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1) != HAL_OK) {
      Error_Handler();
    }
    // TODO? make sure we don't make any unintentional pulse here
    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2) != HAL_OK) {
      Error_Handler();
    }
}
