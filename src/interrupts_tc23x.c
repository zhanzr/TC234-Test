/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Handling of interrupts on TC23x
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#include <machine/wdtcon.h>
#include <machine/intrinsics.h>

#include "tc_inc_path_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)

#include "interrupts_tc23x.h"

extern void _init_vectab(void);
extern int _install_int_handler(int intno, void (*handler)(int), int arg);

static Ifx_SRC_SRCR_Bits * const tabSRC = (Ifx_SRC_SRCR_Bits *)&MODULE_SRC;

/*---------------------------------------------------------------------
	Function:	InterruptInit
	Purpose:	Initialisation of interrupt handling
	Arguments:	void
	Return:		void
---------------------------------------------------------------------*/
void InterruptInit(void)
{
	/* basic initialisation of vector tables */
	_init_vectab();

	/* enable external interrupts */
	_enable();
}

/*---------------------------------------------------------------------
	Function:	InterruptInstall
	Purpose:	Install a service handler for an interrupt
	Arguments:	int irqNum       - number of interrupt
				isrhnd_t isrProc - pointer to service routine
				int prio         - priority (1-255)
				int arg          - argument for service routine
	Return:		void
---------------------------------------------------------------------*/
void InterruptInstall(int irqNum, isrhnd_t isrProc, int prio, int arg)
{
	unsigned int coreId = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;

	if ((irqNum < 0) || (IRQ_ID_MAX_NUM <= irqNum))
	{
		return;
	}

	/* install the service routine */
	_install_int_handler(prio, isrProc, arg);

	/* set processor and priority values */
	tabSRC[irqNum].TOS = coreId;
	tabSRC[irqNum].SRPN = prio;
	/* ... and enable it */
	tabSRC[irqNum].SRE = 1;
}
