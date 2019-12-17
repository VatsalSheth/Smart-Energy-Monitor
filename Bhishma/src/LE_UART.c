/*
 * LE_UART.c
 *
 *  Created on: Dec 2, 2019
 *      Author: vkshe
 */

#include "LE_UART.h"

void LEUART_Init_Config(void)
{
	/* Enable clock for GPIO */
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* Enable clock for LEUART0 */
	CMU_ClockEnable(cmuClock_LEUART0, true);
	CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1);

	/* Initialize LEUART0 TX and RX pins */
	GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortA, 2, gpioModeInput, 1);

	LEUART_Init_TypeDef config = LEUART_INIT_DEFAULT;
	//LEUART_Reset(LEUART0);
	LEUART_Init(LEUART0, &config);

	/* Configuring LEUART I/O */
	/* Set up RX pin */
	LEUART0->ROUTELOC0 = (LEUART0->ROUTELOC0 & (~_LEUART_ROUTELOC0_RXLOC_MASK)) | LEUART_ROUTELOC0_RXLOC_LOC1;
	LEUART0->ROUTEPEN = LEUART0->ROUTEPEN | LEUART_ROUTEPEN_RXPEN;

	/* Set up TX pin */
	LEUART0->ROUTELOC0 = (LEUART0->ROUTELOC0 & (~_LEUART_ROUTELOC0_TXLOC_MASK)) | LEUART_ROUTELOC0_TXLOC_LOC3;
	LEUART0->ROUTEPEN = LEUART0->ROUTEPEN | LEUART_ROUTEPEN_TXPEN;
}
