/*
 * clk_output.h
 *
 *  Created on: Dec 13, 2019
 *      Author: jains
 */

#ifndef SRC_CLK_OUTPUT_H_
#define SRC_CLK_OUTPUT_H_

#include "em_cmu.h"

#define LFXO							(0x03)
#define HFXO							(0x06)
#define CLKOUTSEL						(0x1F)

#define CLKOUTSEL0(x)					(x)
#define CLKOUTSEL1(x)					(x << 5)

#define CMU_ROUTELOC0_CLKOUTLOC_MASK	(0x3F)
#define CMU_ROUTELOC0_CLKOUT0LOC(x)		(x)
#define CMU_ROUTELOC0_CLKOUT1LOC(x)		(x << 8)

#define CMU_ROUTELOC_LOC2				(0x02)

void clock_output(void);

#endif /* SRC_CLK_OUTPUT_H_ */
