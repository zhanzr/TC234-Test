#include "timer.h"

volatile uint32_t FreeRTOSRunTimeTicks;

const uint32_t GetFreeRTOSRunTimeTicks(void)
{
	return FreeRTOSRunTimeTicks;
}

void prvStatTickHandler(uint32_t reload_value)
{
	MODULE_STM0.ISCR.B.CMP1IRR = 1;
	MODULE_STM0.CMP[1].U += (uint32_t)reload_value;

	++FreeRTOSRunTimeTicks;
}

void ConfigureTimeForRunTimeStats(void)
{
	FreeRTOSRunTimeTicks = 0;

	InterruptInstall(SRC_ID_STM0SR1, prvStatTickHandler, configRUNTIME_STAT_INTERRUPT_PRIORITY, CMP1_MATCH_VAL);

	/* Determine how many bits are used without changing other bits in the CMCON register. */
	MODULE_STM0.CMCON.B.MSIZE1 &= ~0x1fUL;
	MODULE_STM0.CMCON.B.MSIZE1 |= (0x1f - __builtin_clz( CMP1_MATCH_VAL));

	/* reset interrupt flag */
	MODULE_STM0.ISCR.B.CMP1IRR = 0;
	/* prepare compare register */
	MODULE_STM0.CMP[1].U = MODULE_STM0.TIM0.U + CMP1_MATCH_VAL;

	MODULE_STM0.ICR.B.CMP1OS = 1;
	MODULE_STM0.ICR.B.CMP1EN = 1;
}
