/*
 * wiznet_spi.c
 *
 *  Created on: 10-Dec-2019
 *      Author: hardi
 */

#include "wiznet_spi.h"


void spi_cs_low()
{

	/*Pull the CS pin low*/
	GPIO_PinOutClear(gpioPortC, SPI1_CS_PIN);
}

void spi_cs_high()
{
	/*Pull the CS pin high*/
	GPIO_PinOutSet(gpioPortC, SPI1_CS_PIN);
}

void spi_write_byte(uint8_t byte)
{

	USART_SpiTransfer(USART1, byte);
	//USART_Tx(USART1, byte);

}


uint8_t spi_read_byte(void)
{

	uint8_t read_data = 0;

	read_data = USART_SpiTransfer(USART1, read_data);
	//read_data = USART_Rx(USART1);

	return read_data;
}


/**
 * @brief this function initializes SPI communication
 * for the communication between MCU and Wiznet chip
 * SPI1_CLK: PC8 Yellow
 * SPI0_CS: PC9 Violet
 * SPI0_MISO: PC7 Grey
 * SPI0_MOSI: PC6 Brown
 */
void spi_init()
{
	//CMU_OscillatorEnable(cmuOsc_HFRCO , true, true);

	/*Clock Configuration*/
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_USART1, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	/*GPIO Port D pin configuration for SPI functionality*/
	GPIO_PinModeSet(gpioPortC, SPI1_MOSI_PIN, gpioModePushPull, PIN_OUT_HIGH);
	GPIO_PinModeSet(gpioPortC, SPI1_MISO_PIN, gpioModeInput, PIN_OUT_LOW);
	GPIO_PinModeSet(gpioPortC, SPI1_CLK_PIN, gpioModePushPull, PIN_OUT_HIGH);
	GPIO_PinModeSet(gpioPortC, SPI1_CS_PIN, gpioModePushPull, PIN_OUT_HIGH);


	/*USART configuration and initialization*/
	USART_InitSync_TypeDef USART1_config = USART_INITSYNC_DEFAULT;

	USART1_config.enable = usartDisable;
	USART1_config.clockMode = usartClockMode0;
	USART1_config.msbf = true;
	USART1_config.autoCsEnable = false;

	USART_InitSync(USART1, &USART1_config);

	/*Enable Tx and Rx*/
	USART1->ROUTELOC0 = USART_ROUTELOC0_CLKLOC_LOC11 | USART_ROUTELOC0_RXLOC_LOC11 | USART_ROUTELOC0_TXLOC_LOC11;
	USART1->ROUTEPEN = USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;

	USART_Enable(USART1, usartEnable);

}
