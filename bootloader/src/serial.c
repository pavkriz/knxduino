#include "serial.h"
#include "stm32g0xx.h"
#include "stm32g0xx_hal.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DUMP_TELEGRAMS

static uint8_t TxBuffer[TXBUFSIZE];
static uint32_t TxHead = 0;
static uint32_t TxTail = 0;	// first empty byte in buffer

UART_HandleTypeDef debug_huart;
DMA_HandleTypeDef hdma_uart2_tx;


/* UART2 Interrupt Service Routine */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&debug_huart);
}


//Transmits data from Tx circular buffer either in one part if data is linear or in two parts if data is broken by circular buffers end.
static void TransmitTxBuffer(void){

    HAL_NVIC_DisableIRQ(USART1_IRQn);                                            //DMA interrupt must be disabled - if transfer is complete immediately (one byte only, DMA transmit function invokes interrupt before tail position is updated)
    if( (TxHead != TxTail) && debug_huart.gState == HAL_UART_STATE_READY) {                                                        //If there is data to transfer and no tranmission in progress
        if( TxHead > TxTail ){                                                    //If all data in circular buffer is linear
            if( HAL_UART_Transmit_IT(&debug_huart, TxBuffer + TxTail , TxHead - TxTail) == HAL_OK ){
                TxTail = TxHead;                                                //Transmit all data and move tail to head position
            }
        }
        else{                                                                    //If data is separated by circular buffer end
            if( HAL_UART_Transmit_IT(&debug_huart, TxBuffer + TxTail , TXBUFSIZE - TxTail) == HAL_OK ){
                TxTail = 0;                                                        //Transmit first end of buffer data part and then linear part from beginning of circular buffer
            }
        }
    }
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if( huart == &debug_huart ){
        TransmitTxBuffer();
    }
}

#endif

void UART_printf(const char *fmt, ...) {
#ifdef DUMP_TELEGRAMS
	char textbuf[2000]; //string buffer

	va_list args;
	va_start(args, fmt);
	vsprintf(textbuf, fmt, args);
	va_end(args);
	//HAL_UART_Transmit(&huart2,textbuf, strlen(textbuf), 500); // ms timeout

	uint32_t i;

	for( i=0; i<strlen(textbuf); i++ ){
		TxBuffer[ TxHead++ ] = textbuf[i];
		TxHead &= TXBUFHEADMASK;
	}

	TransmitTxBuffer();
#endif
}

void serial_setup() {
#ifdef DUMP_TELEGRAMS
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	/**USART1 GPIO Configuration    
	    PA9     ------> USART1_TX
	    PA10     ------> USART1_RX 
	    */
	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	__HAL_RCC_USART1_CLK_ENABLE();

	debug_huart.Instance = USART1;
	debug_huart.Init.BaudRate = 115200;
	debug_huart.Init.WordLength = UART_WORDLENGTH_8B;
	debug_huart.Init.StopBits = UART_STOPBITS_1;
	debug_huart.Init.Parity = UART_PARITY_NONE;
	debug_huart.Init.Mode = UART_MODE_TX_RX;
	debug_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	debug_huart.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&debug_huart);

	/* Peripheral interrupt init*/
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
#endif
}
