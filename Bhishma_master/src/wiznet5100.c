/*
 * wiznet5100.c
 *
 *  Created on: 25-Oct-2019
 *      Author: hardik
 */

#include "wiznet5100.h"
#include "wizchip_conf.h"
#include "socket.h"

#define GREETING_MSG 		 "Well done guys! Welcome\r\n"

void wiznet5100_init()
{
	uint8_t retVal = 0, sockStatus;
	uint8_t rxtx_bufsize[] = { 2, 2, 2, 2 };
	wiz_NetInfo netInfo =
	{
			.mac = { 0x94, 0xE9, 0x79, 0xF2, 0x62, 0x84 },	// Mac address
			.ip = { 128, 138, 189, 149 },					// IP address
			.sn = { 255, 255, 255, 0 },					// Subnet mask
			.gw = { 128, 138, 189, 1 } };					// Gateway address

	spi_init();

	reg_wizchip_cs_cbfunc(spi_cs_low, spi_cs_high);
	reg_wizchip_spi_cbfunc(spi_read_byte, spi_write_byte);

	wizchip_init(rxtx_bufsize, rxtx_bufsize);

	wizchip_setnetinfo(&netInfo);

	reconnect:
	/* Open socket 0 as TCP_SOCKET with port 5000 */
	if ((retVal = socket(0, Sn_MR_TCP, 9747, 0)) == 0)
	{
		uint8_t dipr_address[] = { 128, 138, 189, 162 };

		connect(0, dipr_address, 9000);

		/* OK. Got a remote peer. Let's send a message to it */
		while (1)
		{
			printf("Return Value: %x\n\r", getSn_SR(0));
			for (int i = 0; i < 1000; i++);
			/* If connection is ESTABLISHED with remote peer */
			if ((sockStatus = getSn_SR(0)) == SOCK_ESTABLISHED)
			{
				uint8_t remoteIP[4];
				uint16_t remotePort;
				/* Retrieving remote peer IP and port number */
				getsockopt(0, SO_DESTIP, remoteIP);
				getsockopt(0, SO_DESTPORT, (uint8_t*) &remotePort);
				printf("Connection Established\n\r");
				printf("%d.%d.%d.%d\n\r", remoteIP[0], remoteIP[1],
										  remoteIP[2], remoteIP[3]);
				/* Let's send a welcome message and closing socket */
				if ((retVal = send(0, GREETING_MSG, strlen(GREETING_MSG))) == (int16_t) strlen(GREETING_MSG))
				{
					printf("Sent I think");
				}
				else
				{ /* Ops: something went wrong during data transfer */
					printf("Something Went Wrong\n\r");
				}
				//					break;
			}
			else if ((sockStatus = getSn_SR(0)) == SOCK_CLOSED)
			{ /* Something went wrong with remote peer, maybe the connection was closed unexpectedly */
				printf("Time Out, No Response from Server\n\r");
				break;
			}
			else
			{
				printf("Connection Closed Unexpect\n\r");
				printf("Status Value: %x\n\r", getSn_SR(0));
				break;
			}
		}

	}
	else
	{ /* Can't open the socket. This means something is wrong with W5100 configuration: maybe SPI issue? */
		printf("Weird Issuess\n\r");
	}

	/* We close the socket and start a connection again */
	disconnect(0);
	close(0);
	goto reconnect;
}
