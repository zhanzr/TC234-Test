/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Hardware-dependent module providing a time base
*           by programming a system timer (TriCore TC23xx version).
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#include <machine/wdtcon.h>
#include <machine/intrinsics.h>
#include "interrupts.h"
#include "system_tc2x.h"

#include "tc_inc_path.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)

#include "timer.h"

#define SYSTIME_ISR_PRIO	2

static Ifx_STM * const StmBase = (Ifx_STM *)&MODULE_STM0;


/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value = 0;

/* pointer to user specified timer callback function */
static TCF user_handler = (TCF)0;

/* timer interrupt routine */
static void tick_irq(int reload_value)
{
	/* set new compare value */
	StmBase->CMP[0].U += (unsigned int)reload_value;
	if (user_handler)
	{
		user_handler();
	}
}

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz)
{
	unsigned int frequency = SYSTEM_GetStmClock();
	int irqId = SRC_ID_STM0SR0;

	reload_value = frequency / hz;

	/* install handler for timer interrupt */
	InterruptInstall(irqId, tick_irq, SYSTIME_ISR_PRIO, (int)reload_value);

	/* prepare compare register */
	StmBase->CMP[0].U = StmBase->TIM0.U + reload_value;
	StmBase->CMCON.B.MSIZE0 = 31;	/* use bits 31:0 for compare */
	/* reset interrupt flag */
	StmBase->ISCR.U = (IFX_STM_ISCR_CMP0IRR_MSK << IFX_STM_ISCR_CMP0IRR_OFF);
	StmBase->ICR.B.CMP0EN = 1;
}

/* Install <handler> as timer callback function */
void TimerSetHandler(TCF handler)
{
	user_handler = handler;
}
