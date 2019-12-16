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

enum socket
{
	SOCKET_0,
	SOCKET_1,
	SOCKET_2,
	SOCKET_3
};


#define ADDRESS_MASK_MSB					(0xFF00)
#define ADDRESS_MASK_LSB					(0x00FF)

#define ADDRESS_SHIFT_TO_LSB				(8)

#define MAC_ADDR_SIZE						(6)
#define IP_ADRESS_SIZE						(4)
#define GW_ADDRESS_SIZE						(4)
#define SUBNET_ADDRESS_SIZE					(4)

#define TRUE								(1)
#define FALSE								(0)


typedef struct
{
	uint8_t mac_address[MAC_ADDR_SIZE];
	uint8_t ip_address[IP_ADRESS_SIZE];
	uint8_t subnet_mask[SUBNET_ADDRESS_SIZE];
	uint8_t gateway[GW_ADDRESS_SIZE];
}wiznet_netinfo_t;

void wiznet5100_init(void);
void setup_net_addresses(wiznet_netinfo_t *wiznet5100);
void wiznet_basic_config(wiznet_netinfo_t *wiznet5100);
void set_getway_address(wiznet_netinfo_t *wiznet5100);
void set_mac_address(wiznet_netinfo_t *wiznet5100);
void set_subnet_mask(wiznet_netinfo_t *wiznet5100);
void set_ip_address(wiznet_netinfo_t *wiznet5100);
uint8_t socket_initialization(uint8_t socket);
uint8_t listen_socket(uint8_t socket);
void close_socket(uint8_t socket);
uint8_t connect_server(uint8_t socket);
void set_rx_memory_size(void);
void set_tx_memory_size(void);




#endif /* EXTERNAL_GENERATED_FILES_WIZNET5100_H_ */
