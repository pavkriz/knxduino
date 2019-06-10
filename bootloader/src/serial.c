#include "serial.h"
#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include <stdarg.h>

#ifdef DUMP_TELEGRAMS

static uint8_t TxBuffer[TXBUFSIZE];
static uint32_t TxHead = 0;
static uint32_t TxTail = 0;	// first empty byte in buffer

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_uart2_tx;


/* UART2 Interrupt Service Routine */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}


//Transmits data from Tx circular buffer either in one part if data is linear or in two parts if data is broken by circular buffers end.
static void TransmitTxBuffer(void){

    HAL_NVIC_DisableIRQ(USART2_IRQn);                                            //DMA interrupt must be disabled - if transfer is complete immediately (one byte only, DMA transmit function invokes interrupt before tail position is updated)
    if( (TxHead != TxTail) && huart2.gState == HAL_UART_STATE_READY) {                                                        //If there is data to transfer and no tranmission in progress
        if( TxHead > TxTail ){                                                    //If all data in circular buffer is linear
            if( HAL_UART_Transmit_IT(&huart2, TxBuffer + TxTail , TxHead - TxTail) == HAL_OK ){
                TxTail = TxHead;                                                //Transmit all data and move tail to head position
            }
        }
        else{                                                                    //If data is separated by circular buffer end
            if( HAL_UART_Transmit_IT(&huart2, TxBuffer + TxTail , TXBUFSIZE - TxTail) == HAL_OK ){
                TxTail = 0;                                                        //Transmit first end of buffer data part and then linear part from beginning of circular buffer
            }
        }
    }
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if( huart == &huart2 ){
        TransmitTxBuffer();
    }
}

#endif

void UART2_printf(const char *fmt, ...) {
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

	/**USART2 GPIO Configuration
	  PA2     ------> USART2_TX
	  PA3     ------> USART2_RX
	 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	__HAL_RCC_USART2_CLK_ENABLE();

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart2);

	/* Peripheral interrupt init*/
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
#endif
}
