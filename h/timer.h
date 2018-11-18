/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Hardware-dependent module providing a time base
*           by programming a system timer
*
* Copyright HighTec EDV-Systeme GmbH 1982-2015
*====================================================================*/

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#include <machine/wdtcon.h>
#include <machine/intrinsics.h>
#include "interrupts_tc23x.h"
#include "system_tc2x.h"

#include "tc_inc_path_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)

#include "portmacro.h"
#include "FreeRTOSConfig.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define CMP1_MATCH_VAL	( configPERIPHERAL_CLOCK_HZ /  configRUN_TIME_STATS_RATE_HZ)

const uint32_t GetFreeRTOSRunTimeTicks(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIMER_H__ */
