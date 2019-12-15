/*
 * calendar.h
 *
 *  Created on: 2016. okt. 13.
 *      Author: arkalvac
 */

#include <stdint.h>
#include <stdio.h>
#include "native_gecko.h"

#ifndef CALENDAR_H_
#define CALENDAR_H_

#include "bg_types.h"

PACKSTRUCT (struct calendar_maintain
{
uint16_t cal_year;
uint8_t cal_month;
uint8_t cal_day;
uint8_t cal_weekday;
uint8_t cal_hour;
uint8_t cal_min;
uint8_t cal_sec;
uint16_t cal_ms;
uint16_t cal_tick;
int8_t  cal_time_zone;
uint8_t cal_dst;
uint8 para_slot_len;
uint8 para_slot_data[];
}) maintain;

void updateCalendar();
void setDateAndTime(uint16_t year, uint8_t month, uint8_t day, uint8_t weekday, uint8_t hour, uint8_t min, uint8_t sec, uint16_t ms);
void getDateAndTime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* weekday, uint8_t* hour, uint8_t* min, uint8_t* sec, uint16_t* ms);
void setTimeZone(int8_t time_zone);
int8_t getTimeZone();
void setDst(uint8_t dst);
uint8_t getDst();

void ps_save(void);
void ps_load(void);

#endif /* CALENDAR_H_ */
