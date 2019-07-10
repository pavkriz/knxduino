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

    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              uwTimclock, uwAPB1Prescaler = 0U;
    uint32_t              uwPrescalerValue = 0U;
    uint32_t              pFLatency;
    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    /* Get APB1 prescaler */
    uwAPB1Prescaler = clkconfig.APB1CLKDivider;
    /* Compute TIM clock */
    if (uwAPB1Prescaler == RCC_HCLK_DIV1) {
      uwTimclock = HAL_RCC_GetPCLK1Freq();
    } else {
      uwTimclock = 2U*HAL_RCC_GetPCLK1Freq();
    }
    /* Compute the prescaler value to have TIM counter clock equal to 1MHz */
    uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

    htim->Instance = _timer.timer;
    htim->Init.Prescaler = uwPrescalerValue; // 72MHz/72 = 1MHz (ie. 1us tick)
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
