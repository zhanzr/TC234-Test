#include "timer.h"

volatile uint32_t g_sys_ticks;
volatile uint32_t g_cmp1_ticks;

const uint32_t HAL_GetTick(void)
{
	return g_sys_ticks;
}

const uint32_t HAL_GetRunTimeTick(void)
{
	return g_cmp1_ticks;
}

/* timer interrupt routine */
void stm_cmp0_isr(uint32_t reload_value)
{
	MODULE_STM0.ISCR.B.CMP1IRR = 1;
	MODULE_STM0.CMP[0].U += (uint32_t)reload_value;

	++g_sys_ticks;
}

void stm_cmp1_isr(uint32_t reload_value)
{
	MODULE_STM0.ISCR.B.CMP0IRR = 1;
	MODULE_STM0.CMP[1].U += (uint32_t)reload_value;

	++g_cmp1_ticks;
}

/* Initialise timer at rate <hz> */
void stm_init(uint8_t ch, uint32_t hz)
{
	uint32_t freq_stm = SYSTEM_GetStmClock();

	uint32_t reload_value = freq_stm / hz;

	if(0==ch)
	{
		InterruptInstall(SRC_ID_STM0SR0, stm_cmp0_isr, STM_CMP0_ISR_PRIO, reload_value);

		/* Determine how many bits are used without changing other bits in the CMCON register. */
		MODULE_STM0.CMCON.B.MSIZE0 &= ~0x1fUL;
		MODULE_STM0.CMCON.B.MSIZE0 |= (0x1f - __builtin_clz( reload_value));

		/* reset interrupt flag */
		MODULE_STM0.ISCR.B.CMP0IRR = 1;
		/* prepare compare register */
		MODULE_STM0.CMP[0].U = MODULE_STM0.TIM0.U + reload_value;

		MODULE_STM0.ICR.B.CMP0OS = 0;
		MODULE_STM0.ICR.B.CMP0EN = 1;
	}
	else
	{
		InterruptInstall(SRC_ID_STM0SR1, stm_cmp1_isr, STM_CMP1_ISR_PRIO, reload_value);

		/* Determine how many bits are used without changing other bits in the CMCON register. */
		MODULE_STM0.CMCON.B.MSIZE1 &= ~0x1fUL;
		MODULE_STM0.CMCON.B.MSIZE1 |= (0x1f - __builtin_clz( reload_value));

		/* reset interrupt flag */
		MODULE_STM0.ISCR.B.CMP1IRR = 1;
		/* prepare compare register */
		MODULE_STM0.CMP[1].U = MODULE_STM0.TIM0.U + reload_value;

		MODULE_STM0.ICR.B.CMP1OS = 1;
		MODULE_STM0.ICR.B.CMP1EN = 1;
	}
}
