/*
 * wiznet_spi.h
 *
 *  Created on: 10-Dec-2019
 *      Author: hardi
 */


#include "em_gpio.h"
#include "em_usart.h"
#include "wiznet5100.h"

#ifndef WIZNET_SPI_H_
#define WIZNET_SPI_H_

#define PIN_OUT_HIGH				(1)
#define PIN_OUT_LOW					(0)


#define SPI1_CLK_PIN				(8)
#define SPI1_CS_PIN					(9)
#define SPI1_MISO_PIN				(7)
#define SPI1_MOSI_PIN				(6)

void spi_init(void);
void spi_cs_low(void);
void spi_cs_high(void);
void spi_write_byte(uint8_t byte);
uint8_t spi_read_byte(void);


#endif /* WIZNET_SPI_H_ */
