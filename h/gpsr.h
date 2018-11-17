/*
 * gpsr.h
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#ifndef GPSR_H_
#define GPSR_H_

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxInt_reg.h)
#include TC_INCLUDE(TCPATH/IfxInt_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)

void config_gpsr(void);

uint8_t test_trigger_gpsr(uint8_t t);
void test_proc_gpsr(void);

#endif /* GPSR_H_ */
