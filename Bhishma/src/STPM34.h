/*
 * STPM34.h
 *
 *  Created on: Dec 3, 2019
 *      Author: vkshe
 */

#ifndef STPM34_H_
#define STPM34_H_

#endif /* STPM34_H_ */

#include "LE_UART.h"
#include "stdio.h"
#include "em_core.h"

#define FRAME_LENGTH (4)
#define SYN (5)
#define SCS (4)

#define Node_ID (0)
#define Power_Type (2)
#define Current_Type (1)
#define Voltage_Type (3)

struct packet
{
	uint8_t Read_addr;
	uint8_t Write_addr;
	uint16_t Write_data;
};

uint32_t RxTx_Frame(struct packet data);
void Disable_CRC();
void SW_Reset();
void Global_Reset();
void Auto_Latch();
void Ch_config();
void stpm34_init();
uint16_t Get_Voltage();
uint16_t Get_Current();
uint16_t Get_Power();


