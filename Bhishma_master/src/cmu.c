//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void cmu_init(void) {

	CMU_OscillatorEnable(cmuOsc_HFXO , true, true);// HFXO Oscillator Enable
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	CMU_OscillatorEnable(cmuOsc_LFXO , true, true);// LFXO Oscillator Enable
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true); //ULFRCO Oscillator Enable

	/* Enable LE domain registers */
	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_ClockEnable(cmuClock_LFA, true);

	/* Enable RTC clock */
	CMU_ClockEnable(cmuClock_RTCC, true);

	// Peripheral clocks enabled
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Enable LFXO as LFACLK in CMU. This will also start LFXO */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO);
}
