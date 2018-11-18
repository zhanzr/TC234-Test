/*====================================================================
* Project:  Board Support Package (BSP)
* Function: CORE startup and initialisation function
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#ifndef __TRICORE_NAME__
#ifdef __TC23XX__
#define __TRICORE_NAME__	0x2300
#else
#error "TriCore derivative is not defined"
#endif /* __TC23XX__ */
#endif /* __TRICORE_NAME__ */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <machine/intrinsics.h>

#include "tc_inc_path_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)

#include "core_tc23x.h"

/* the initial trap table */
void first_trap_table(void) __attribute__ ((noreturn));


void _start(void);
void cstart(const CoreInit_t *) __attribute__ ((interrupt,noinline));
static void WDT_ClearEndinit(volatile unsigned int *wdtbase) __attribute__ ((interrupt,noinline));
static void WDT_SetEndinit(volatile unsigned int *wdtbase) __attribute__ ((interrupt,noinline));
static void clear_table_func(const ClearTable_t *table) __attribute__ ((interrupt));
static void copy_table_func(const CopyTable_t *table) __attribute__ ((interrupt));
static void init_csa(unsigned int csa_base, unsigned int csa_size) __attribute__ ((interrupt));


/* declare symbol used by linker for memory map checking */
#define _STRINGIFY(x)	#x

#define DECL_SYM(name,val)							\
	__asm (".global       " #name );				\
	__asm (".set  " #name "," _STRINGIFY(val));		\
	__asm (".type " #name ",@object");

DECL_SYM(__TRICORE_DERIVATE_NAME__,__TRICORE_NAME__);


#pragma section ".startup.bmhd" ax 4
/* the BMI header description at start of internal flash at 0xa0000000 */
/*********************************************************************************
 * startup code
 *********************************************************************************/
void _RESET(void)
{
	__asm (".global _START");
	__asm (".word 0x00000000");
	__asm (".word 0xb3590070");
	__asm (".word 0x00000000");
	__asm (".word 0x00000000");
	__asm (".word 0x00000000");
	__asm (".word 0x00000000");
	__asm (".word 0x791eb864");
	__asm (".word 0x86e1479b");
	/* we must make a jump to cached segment, why trap_tab follow */
	__asm ("_START: movh.a %a15,hi:_start");
	__asm ("  lea  %a15,[%a15]lo:_start");
	__asm ("  ji %a15");
}
#pragma section

extern void __CSA_BEGIN(void);
extern void __CSA_SIZE(void);
extern void __USTACK(void);
extern void __ISTACK(void);

/* provide compiler information: symbols are not small addressable */
#pragma section ".data" aw
extern const ClearTable_t	__clear_table;
extern const CopyTable_t	__copy_table;

extern unsigned long		__ISTACK_CPU0_;
extern unsigned long		__USTACK_CPU0_;
extern unsigned long		_SMALL_DATA__CPU0_ __attribute__ ((weak));
extern unsigned long		_SMALL_DATA2__CPU0_ __attribute__ ((weak));
extern unsigned long		_SMALL_DATA3__CPU0_ __attribute__ ((weak));
extern unsigned long		_SMALL_DATA4__CPU0_ __attribute__ ((weak));
extern int init_applproc0(int, char **);
#pragma section

/* provide compiler information: symbols are not small addressable */
#pragma section ".rodata" a
const CoreInit_t CPUInit[1] =
{
	[0] =
	{
		.cleartable	= &__clear_table,
		.copytable	= &__copy_table,
		.istack		= (uint32_t)__ISTACK,
		.ustack		= (uint32_t)__USTACK,
		.smallA0	= &_SMALL_DATA__CPU0_,
		.smallA1	= &_SMALL_DATA2__CPU0_,
		.smallA8	= &_SMALL_DATA3__CPU0_,
		.smallA9	= &_SMALL_DATA4__CPU0_,
		.csaBase	= (uint32_t)__CSA_BEGIN,
		.csaSize	= (uint32_t)__CSA_SIZE,
		.wdtCon0	= &SCU_WDTCPU0CON0,
		.wdtCon1	= &SCU_WDTCPU0CON1,
		.main		= init_applproc0,
	}
};
#pragma section

#pragma section ".startup.code" ax 4
void _start(void)
{
	unsigned int coreID = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	const CoreInit_t *core;		/* the core initialisation structure */
	core = &CPUInit[coreID];

	/* load stack pointer (reserve 4 ints for startup) */
	__asm volatile ("mov.aa %%sp, %0" : : "a" (&core->ustack[-4]) : "a10");
	cstart(core);
}
#pragma section

#pragma section ".startup.code" ax 4
/*
 * the initial startup routine
 * - initialise the global RAM
 * - start the other CPUs
 * - disable/enable System Watchdog
 * - call the main function
 */
void cstart(const CoreInit_t *core)
{
	unsigned int psw;

	/* reset endinit and disable watchdog */
	if (core->wdtCon0->U & 1)
	{
		/* clear endinit bit in CPU0 WDT */
		WDT_ClearEndinit(&core->wdtCon0->U);
	}
	if ((core == &CPUInit[0]) && (*&SCU_WDTSCON0.U & 1))
	{
		/* clear endinit bit in safety WDT */
		WDT_ClearEndinit(&SCU_WDTSCON0.U);
	}

	/* setup interrupt stack */
	_mtcr(CPU_ISP, (unsigned int)core->istack);

	/* install trap handlers */
	_mtcr(CPU_BTV, (unsigned int)first_trap_table);
	_isync();

	/* initialise call depth counter */
	psw  = _mfcr(CPU_PSW);
	psw |= IFX_CPU_PSW_CDC_MSK;
	psw &= ~(IFX_CPU_PSW_CDE_MSK << IFX_CPU_PSW_CDE_OFF);
	_mtcr(CPU_PSW, psw);
	_isync();

	/* enable write access to system global registers */
	psw  = _mfcr(CPU_PSW);
	psw |= (IFX_CPU_PSW_GW_MSK << IFX_CPU_PSW_GW_OFF);
	_mtcr(CPU_PSW, psw);
	_isync();

	/* initialise SDA base pointers */
	__asm volatile ("mov.aa %%a0, %0" : : "a" (core->smallA0));
	__asm volatile ("mov.aa %%a1, %0" : : "a" (core->smallA1));
	__asm volatile ("mov.aa %%a8, %0" : : "a" (core->smallA8));
	__asm volatile ("mov.aa %%a9, %0" : : "a" (core->smallA9));

	/* disable write access to system global registers */
	psw  = _mfcr(CPU_PSW);
	psw &= ~(IFX_CPU_PSW_GW_MSK << IFX_CPU_PSW_GW_OFF);
	_mtcr(CPU_PSW, psw);
	_isync();

	/* disable Watchdogs */
	if (core == &CPUInit[0])
	{
		/* safety WDT handled by CPU0 */
		SCU_WDTSCON1.B.DR = 1;
		WDT_SetEndinit(&SCU_WDTSCON0.U);
	}

	core->wdtCon1->B.DR = 1;
	WDT_SetEndinit(&core->wdtCon0->U);

	init_csa(core->csaBase, core->csaSize);

	/* handle global clear and copy tables */
	if (core == &CPUInit[0])
	{
		clear_table_func(&__clear_table);
		copy_table_func(&__copy_table);
	}

	/* handle core specific clear and copy tables */
	clear_table_func(core->cleartable);
	copy_table_func(core->copytable);

	/* pass coreID to main */
	core->ustack[-2] = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	/* terminate argv */
	core->ustack[-1] = 0;
	__asm volatile ("mov.aa %%sp, %0" : : "a" (&core->ustack[-2]) : "a10");
	/* call main program, pass return code to C99 function _Exit */
	/* _Exit is exit without atexit handling (reduced C library overhead) */
	_Exit((*core->main)(1, (char **)&core->ustack[-2]));
}

static void WDT_ClearEndinit(volatile unsigned int *wdtbase)
{
	unsigned int passwd;

	passwd = *wdtbase;
	passwd &= 0xffffff00;
	*wdtbase = passwd | 0xf1;
	*wdtbase = passwd | 0xf2;
	/* read back new value ==> synchronise LFI */
	(void)*wdtbase;
}
static void WDT_SetEndinit(volatile unsigned int *wdtbase)
{
	unsigned int passwd;

	passwd = *wdtbase;
	passwd &= 0xffffff00;
	*wdtbase = passwd | 0xf1;
	*wdtbase = passwd | 0xf3;
	/* read back new value ==> synchronise LFI */
	(void)*wdtbase;
}

static void clear_table_func(const ClearTable_t *pt)
{
	while (pt->size >= 0)
	{
		memset(pt->base, 0l, pt->size);
		pt++;
	}
}

static void copy_table_func(const CopyTable_t *pt)
{
	while (pt->size >= 0)
	{
		memcpy(pt->dst, pt->src, pt->size);
		pt++;
	}
}

#define CSA_FRAME_SIZE	64
#define CSA_ALIGNMENT	(CSA_FRAME_SIZE - 1)

static void init_csa(unsigned int csa_base, unsigned int csa_size)
{
	csa_t		*pcsa;
	pcxi_t		pcxi;
	unsigned int current_csa;

	_mtcr(CPU_PCXI, 0);						/* initialise PCXI */

	/* force correct alignment of CSA */
	csa_base = (csa_base + CSA_ALIGNMENT) & ~CSA_ALIGNMENT;
	csa_size = csa_size / CSA_FRAME_SIZE;		/* number of csa entries */

	pcsa = (csa_t *)csa_base;

	pcsa->reg[0] = 0;
	pcsa++;
	current_csa = csa_base;

	pcxi.bits.pcxs = csa_base >> 28;			/* segment of csa area */
	pcxi.bits.pcxo = ((csa_base + sizeof(csa_t)) >> 6 ) & 0xffff;	/* get csa index */

	_mtcr(CPU_LCX, pcxi.reg);					/* initialise LCX */

	csa_size -= 1;								/* CSA's to initialise -= 1 */

	while (csa_size--)
	{
		pcxi.bits.pcxo = (current_csa >> 6) & 0xffff;
		pcsa->reg[0] = pcxi.reg;
		pcsa++;
		current_csa += sizeof(csa_t);
	}
	pcxi.bits.pcxo = (current_csa >> 6) & 0xffff;
	_mtcr(CPU_FCX, pcxi.reg);					/* initialise FCX */
}
#pragma section


#pragma section ".text" ax
void Start_Core(unsigned int coreId)
{
	/* there are no other cores */
	(void)coreId;
}

static void __attribute__ ((used)) trap_function(void)
{
	__asm volatile ("debug" : : : "memory");
}
#pragma section


#pragma section ".traptable" ax 256
#define TRAP_ENTRY(hnd)						\
 __asm (".align 5\n\t"						\
		"svlcx\n\t"							\
		"debug\n\t"							\
		"movh.a  %a15, hi:"#hnd"\n\t"		\
		"lea     %a15, [%a15]lo:"#hnd"\n\t"	\
		"mov     %d4, %d15\n\t"				\
		"ji      %a15\n\t"					\
		"rslcx\n\t"							\
		"rfe\n\t"							\
		"nop\n\t" )

void first_trap_table(void)
{
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	TRAP_ENTRY(trap_function);
	while(1);
}
#pragma section
