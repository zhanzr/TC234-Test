/*
 * data_flash.h
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#ifndef DATA_FLASH_H_
#define DATA_FLASH_H_

#include <stdbool.h>
#include <stdint.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxFlash_reg.h)
#include TC_INCLUDE(TCPATH/IfxFlash_bf.h)
#include TC_INCLUDE(TCPATH/IfxPmu_reg.h)
#include TC_INCLUDE(TCPATH/IfxPmu_bf.h)

#include "interrupts_tc23x.h"
#include "core_tc23x.h"
#include "asm_prototype.h"

void config_dflash(void);

void test_proc_dflash(void);

#endif /* DATA_FLASH_H_ */
