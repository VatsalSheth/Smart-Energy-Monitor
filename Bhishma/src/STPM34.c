/*
 * STPM34.c
 *
 *  Created on: Dec 3, 2019
 *      Author: vkshe
 */

#include "STPM34.h"

//Return Read of previous frame
uint32_t RxTx_Frame(struct packet data)
{
	uint8_t pk[4];
	uint32_t result = 0, tmp = 0;

	pk[0] = data.Read_addr;
	pk[1] = data.Write_addr;
	pk[2] = (uint8_t)(data.Write_data & 0xff);
	pk[3] = (uint8_t)((data.Write_data & 0xff00)>>8);

	for(int i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}
	for(int i=0; i<10;i++);

	for(uint32_t i=0; i<FRAME_LENGTH; i++)
	{
		tmp = LEUART_Read_byte();
		tmp = tmp << (8*i);
		result |= tmp;
		tmp = 0;
	}

//	printf("%x\n",result);
	return result;
}

void stpm34_init()
{
	LEUART_Init_Config();

	Global_Reset();
	Disable_CRC();
	Auto_Latch();
	Ch_config();
	//SW_Reset();

	for(uint32_t j=0; j<1000; j++);

}

uint16_t Get_Voltage()
{
	struct packet data;
	uint32_t rc, ret;
	float voltage;
	data.Read_addr = 0x48;
	data.Write_addr = 0xff;
	data.Write_data = 0xffff;
	do
	{
		rc = RxTx_Frame(data);
		rc = RxTx_Frame(data);
		voltage = (rc & 0x7fff) * (1.2 * (1 + 810000/470)/(0.875 * 65536 ));
	}while(voltage < 110);
	ret = (Voltage_Type<<14) | (Node_ID << 13);
	ret |= (((int)voltage & 0xff)<<5);
	ret |= ((int)((voltage - (int)voltage)*10) & 0x1f);
	return (uint16_t)ret;
}

uint16_t Get_Current()
{
	struct packet data;
	uint32_t rc, ret = 0;
	float current;

	data.Read_addr = 0x48;
	data.Write_addr = 0xff;
	data.Write_data = 0xffff;
	rc = RxTx_Frame(data);

	rc = RxTx_Frame(data);
	current = (rc >> 15) * (1.2 * (1 + 810000/470)/(0.875 * 65536 ));
	ret = (Current_Type<<14) | (Node_ID << 13);
	ret |= (((int)current & 0x1f)<<8);
	ret |= ((int)((current - (int)current)*100) & 0xff);
	printf("Current from API: %d\r\n", (100*current));
	return (uint16_t)ret;
}

uint16_t Get_Power()
{
	struct packet data;
	uint32_t power, ret = 0;
	data.Read_addr = 0x62;
	data.Write_addr = 0xff;
	data.Write_data = 0xffff;
	power = RxTx_Frame(data);
	power = RxTx_Frame(data);
	ret = (Power_Type<<14) | (Node_ID << 13);
	ret |= power & 0x1fff;
	return (uint16_t)ret;
}

void Global_Reset()
{
	GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 1);

	GPIO_PinOutSet(gpioPortA, SYN);
	GPIO_PinOutSet(gpioPortA, SCS);

	for(uint32_t i=0; i<3; i++)
	{
		for(uint32_t j=0; j<100; j++);
		GPIO_PinOutClear(gpioPortA, SYN);
		for(uint32_t j=0; j<100; j++);
		GPIO_PinOutSet(gpioPortA, SYN);
	}
}

void Ch_config()
{
	uint32_t i;
	uint8_t pk[4];

	pk[0] = 0x01;
	pk[1] = 0x01;
	pk[2] = 0x1e;
	pk[3] = 0x00;

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Read_byte();
	}

	pk[0] = 0x48;
	pk[1] = 0x02;
	pk[2] = 0x80;
	pk[3] = 0x00;

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Read_byte();
	}

}


void SW_Reset()
{
	uint32_t i;
	uint8_t pk[4];

	pk[0] = 0x48;
	pk[1] = 0x05;
	pk[2] = 0x10;
	pk[3] = 0x00;

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Read_byte();
	}

}

void Disable_CRC()
{
	uint32_t i;
	uint8_t pk[5];

	pk[0] = 0x48;
	pk[1] = 0x24;
	pk[2] = 0x07;
	pk[3] = 0x00;
	pk[4] = 0x07;

	for(i=0; i<=FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Read_byte();
	}
}

void Auto_Latch()
{
	uint32_t i;
	uint8_t pk[4];

	pk[0] = 0x05;
	pk[1] = 0x05;
	pk[2] = 0x80;
	pk[3] = 0x00;

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Write_byte(pk[i]);
	}

	for(i=0; i<FRAME_LENGTH; i++)
	{
		LEUART_Read_byte();
	}
}
