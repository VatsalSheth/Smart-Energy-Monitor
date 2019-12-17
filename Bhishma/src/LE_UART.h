/*
 * LE_UART.h
 *
 *  Created on: Dec 2, 2019
 *      Author: vkshe
 */

#ifndef LE_UART_H_
#define LE_UART_H_

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"

#endif /* LE_UART_H_ */

static inline void LEUART_Write_byte(uint8_t data);
static inline uint8_t LEUART_Read_byte();
void LEUART_Init_Config(void);



static inline void LEUART_Write_byte(uint8_t data)
{
	LEUART_Tx(LEUART0, data);
}

static inline uint8_t LEUART_Read_byte()
{
	 return (uint8_t)LEUART_Rx(LEUART0);
}
