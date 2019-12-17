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

uint8_t wiznet5100_init()
{

	uint8_t retVal = 0, sockStatus;
	uint8_t retries = 0;

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
	if ((retVal = socket(SOCKET_0, Sn_MR_TCP, 9867, 0)) == 0)
	{
		uint8_t dipr_address[] = {172, 21, 74, 225};

//		printf("SPI CHECK Return Value: %x\n\r", getSn_SR(0));

		connect(SOCKET_0, dipr_address, 9867);

		/* OK. Got a remote peer. Let's send a message to it */
		while (retries<20)
		{
//			printf("Return Value: %x\n\r", getSn_SR(0));
			for (int i = 0; i < 1000; i++);
			/* If connection is ESTABLISHED with remote peer */
			if ((sockStatus = getSn_SR(SOCKET_0)) == SOCK_ESTABLISHED)
			{
				uint8_t remoteIP[4];
				uint16_t remotePort;
				/* Retrieving remote peer IP and port number */
				getsockopt(SOCKET_0, SO_DESTIP, remoteIP);
				getsockopt(SOCKET_0, SO_DESTPORT, (uint8_t*) &remotePort);
				printf("Connection Established\n\r");
				printf("%d.%d.%d.%d\n\r", remoteIP[0], remoteIP[1], remoteIP[2],
						remoteIP[3]);
				/* Let's send a welcome message and closing socket */
//				if ((retVal = send(SOCKET_0, GREETING_MSG, strlen(GREETING_MSG)))
//						== (int16_t) strlen(GREETING_MSG))
//				{
//					printf("Packet Sent");
//				}
//				else
//				{ /* Ops: something went wrong during data transfer */
//					printf("Error in sending packet\n\r");
//				}
				break;
			}
			else if ((sockStatus = getSn_SR(SOCKET_0)) == SOCK_CLOSED)
			{ /* Something went wrong with remote peer, maybe the connection was closed unexpectedly */
				printf("Time Out, No Response from Server\n\r");
				disconnect(SOCKET_0);
				close(SOCKET_0);
				retries++;
				goto reconnect;
				break;
			}
			else
			{
				printf("Connection Closed Unexpectedly\n\r");
//				printf("Status Value: %x\n\r", getSn_SR(0));
				disconnect(SOCKET_0);
				close(SOCKET_0);
				retries++;
				goto reconnect;
			}
		}
	}
	else
	{ /* Can't open the socket. This means something is wrong with W5100 configuration: maybe SPI issue? */
		printf("Error opening socket\n\r");
		retries=0;
		return 1;
	}
	if(retries == 20)
		return 1;
	else
		return 0;
	/* We close the socket and start a connection again */
//	disconnect(0);
//	close(0);
//	goto reconnect;
}

void send_socket(char* socket_packet)
{
	uint8_t sockStatus = 0, retVal = 0;
	if ((sockStatus = getSn_SR(SOCKET_0)) == SOCK_ESTABLISHED)
	{
		/* Let's send data*/
		if ((retVal = send(SOCKET_0, socket_packet, strlen(socket_packet)))
				== (int16_t) strlen(socket_packet))
		{
			printf("AC data Packet Sent");
		}
		else
		{ /* Oops: something went wrong during data transfer */
			printf("Error in sending AC data packet\n\r");
		}
	}
	else printf("Socket not established in send_socket\r\n");
}
