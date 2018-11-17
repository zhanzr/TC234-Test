/*
 * dts.h
 *
 *  Created on: Nov 17, 2018
 *      Author: zzr
 */

#ifndef DTS_H_
#define DTS_H_

#include <stdbool.h>
#include <stdint.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)

#include "interrupts_tc23x.h"

bool IfxDts_isReady(void);
void config_dts(void);
int16_t read_dts(void);
float read_dts_celsius(void);
void start_dts_measure(void);

void test_proc_dts(void);

#endif /* DTS_H_ */
