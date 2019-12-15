#include "rtcc.h"

void rtcSetup(void)
{
  RTCC_Init_TypeDef rtccInit = //RTCC_INIT_DEFAULT;
  {
  /* Initialize RTC */
  .enable   = true,  /* Start RTC after initialization is complete. */
  .debugRun = false,  /* Halt RTC when debugging. */
  .cntMode = rtccCntModeNormal,

  .precntWrapOnCCV0 = false,
  .cntWrapOnCCV1 = false,
  .prescMode = rtccCntTickPresc,
  .presc = rtccCntPresc_256,
  .enaOSCFailDetect = false,
  };
  RTCC_Init(&rtccInit);

  RTCC_CCChConf_TypeDef rtccCCSetup =
  {
	.chMode = rtccCapComChModeCompare,
	.compMatchOutAction = rtccCompMatchOutActionSet,
	.compBase = rtccCompBaseCnt,
  };

  RTCC_ChannelInit(0, &rtccCCSetup);

//  RMU->CTRL = (RMU->CTRL & (~_RMU_CTRL_SYSRMODE_MASK)) | RMU_CTRL_SYSRMODE_LIMITED;

  NVIC_EnableIRQ(RTCC_IRQn);
  RTCC_IntClear(RTCC_IFC_CC0);
  RTCC_IntEnable(RTCC_IEN_CC0);

  RTCC_ChannelCCVSet(0,640);
}

#ifdef last_ditch
void applicationTask()
{
	const char* weekdays[] = {"MON","TUE","WED","THU","FRI","SAT","SUN"};

	uint16_t year,ms;
	uint8_t month,day,weekday,hour,min,sec;
	uint16_t uphours,upmins,upsec,upms;

	uint32_t uptime = RTCC_CounterGet();

	uphours = uptime/128/60/60 % 24;
	upmins = uptime/128/60 % 60;
	upsec = uptime/128 % 60;
	upms = uptime % 128 * 1000 / 128;

	getDateAndTime(&year,&month,&day,&weekday,&hour,&min,&sec,&ms);

	printf("%4d-%02d-%02d %02d:%02d:%02d.%03d %s   uptime: %02d:%02d:%02d.%03d \r",year,month,day,hour,min,sec,ms,weekdays[weekday-1],uphours,upmins,upsec,upms);
}
#endif

void RTCC_IRQHandler(void)
{
	printf("IRQ received\r\n");
}

uint8_t retrieve_sec()
{
	uint16_t year,ms;
	uint8_t month,day,weekday,hour,min,sec;
	getDateAndTime(&year,&month,&day,&weekday,&hour,&min,&sec,&ms);
	return sec;
}

uint8_t retrieve_minutes()
{
	uint16_t year,ms;
	uint8_t month,day,weekday,hour,min,sec;
	getDateAndTime(&year,&month,&day,&weekday,&hour,&min,&sec,&ms);
	return min;
}

uint8_t retrieve_hours()
{
	uint16_t year,ms;
	uint8_t month,day,weekday,hour,min,sec;
	getDateAndTime(&year,&month,&day,&weekday,&hour,&min,&sec,&ms);
	return hour;
}
