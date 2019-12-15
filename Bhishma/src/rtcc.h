#include "em_rtcc.h"
#include "calendar.h"
#include "src/cmu.h"

//#define last_ditch

void applicationTask(void);
void rtcSetup(void);

uint8_t retrieve_sec(void);
uint8_t retrieve_minutes(void);
uint8_t retrieve_hours(void);
