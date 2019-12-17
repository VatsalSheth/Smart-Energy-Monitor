/*
 * wiznet5100.h
 *
 *  Created on: 25-Oct-2019
 *      Author: hardi
 */

#ifndef EXTERNAL_GENERATED_FILES_WIZNET5100_H_
#define EXTERNAL_GENERATED_FILES_WIZNET5100_H_

#include "em_gpio.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "stdio.h"
#include "wiznet_spi.h"
#include "string.h"

enum socket
{
	SOCKET_0,
	SOCKET_1,
	SOCKET_2,
	SOCKET_3
};


uint8_t wiznet5100_init(void);
void send_socket(char* socket_packet);

#endif /* EXTERNAL_GENERATED_FILES_WIZNET5100_H_ */
