/*
 * scu_eru.h
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#ifndef SCU_ERU_H_
#define SCU_ERU_H_

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

void config_eru(void);

#endif /* SCU_ERU_H_ */
