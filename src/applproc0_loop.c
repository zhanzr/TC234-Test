/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Application Processor 0 function
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#include <stdlib.h>
#include <machine/intrinsics.h>

#include "system_tc2x.h"
#include "tc_inc_path_tc23x.h"
#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCbs_reg.h)


/* link to trap table */
/* all cores will use the same trap table */
extern void first_trap_table(void);

extern int main(void);

int init_applproc0(int argc, char **argv);

/* activate CDC (Core Debug Controller) */
static void enable_cdc(void)
{
	/* check whether CDC is already active */
	if (0 == CBS_OSTATE.B.OEN)
	{
		CBS_OEC.U = 0x100A1;
		CBS_OEC.U = 0x1005E;
		CBS_OEC.U = 0x100A1;
		CBS_OEC.U = 0x1005E;
	}
}


/*!
 * \brief   main_init Function.
 *
 * main_init() will be called after config initialisation
 */
static void main_init(void)
{
	SYSTEM_Init();

	unlock_wdtcon();
	_dsync();
	/* install trap handlers */
	_mtcr(CPU_BTV, (unsigned int)first_trap_table);
	_isync();

	_mtcr(CPU_PCON0, 0);		/* enable code cache */
	_mtcr(CPU_DCON0, 0);		/* enable data cache */
	_dsync();
	lock_wdtcon();

	/* CDC (Core Debug Controller) must be active for performance counter usage */
	enable_cdc();

	/* activate performance counters */
	/* CE = 1, M1 = PCACHE_HIT, M2 = PCACHE_MISS, M3 = DCACHE_MISS_DIRTY (only TC1.6P) */
	_mtcr(CPU_CCTRL, 0x2 | (0x1 << 2) | (0x1 << 5) | (0x2 << 8));
}



int init_applproc0(int argc, char **argv)
{
	int res = 0;

	(void)argc;
	(void)argv;

	/* hw dependent functions */
	main_init();

	/* all global initialisation done, call the main routine for CPU0 */
	res = main();

	return res;
}
