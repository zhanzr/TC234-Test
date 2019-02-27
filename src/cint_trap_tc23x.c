/*====================================================================
 * Project:  Board Support Package (BSP)
 * Function: C interface for TriCore trap and interrupt handlers
 *
 * Copyright HighTec EDV-Systeme GmbH 1982-2016
 *====================================================================*/

#ifndef MODULE_UART_INT
#define MODULE_UART_INT
#endif

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bspconfig_tc23x.h"
#include "uart_int.h"
#include "led.h"

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>

#include "cint_trap_tc23x.h"
#include "tc_inc_path_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)

/* This array holds the functions to be called when a trap occurs. */
void (*Tdisptab[MAX_TRAPS])(int tin);

/* This array holds the functions to be called when an interrupt occurs.  */
Hnd_arg Cdisptab[MAX_INTRS];
char g_tmp_output_buf[1024];

/* Install TRAPHANDLER for trap TRAPNO.  */

/* Send character CHR via the serial line */
static __inline void _out_uart(const char chr) {
	TX_CLEAR(UARTBASE);
	/* send the character */
	PUT_CHAR(UARTBASE, chr);
	simple_delay(10000);
}

static inline void _output_buf(const char* str, uint16_t len) {
	for(uint16_t i=0; i<len; ++i) {
		_out_uart(*(str+i));
	}
}

int _install_trap_handler(int trapno, void (*traphandler)(int)) {
	if ((trapno < 0) || (trapno >= MAX_TRAPS)) {
		return 0;
	}

	Tdisptab[trapno] = traphandler;

	return 1;
}

/* The default handlers for class 0-7 traps; if the CPU is programmed to
   honor debug instructions, program control will return to the debugger
   and you can see which trap was triggered.  */

/* MMU  */

static void __class_0_trap_handler(int tin)
{
	led_on(0);

	_out_uart('<');
	flush_stdout_trap();
	_out_uart('0');
	flush_stdout_trap();
	_out_uart('>');
	flush_stdout_trap();
	_out_uart('[');
	flush_stdout_trap();
	_out_uart('0'+tin);
	flush_stdout_trap();
	_out_uart(']');
	flush_stdout_trap();

	switch (tin)
	{
	case 1:
		_debug(); /* VAF -- Virtual Address Fill  */
		break;

	case 2:
		_debug(); /* VAP -- Virtual Address Protection  */
		break;

	default:
		_debug();
		break;
	}
}

/* Internal Protection traps  */

static void __class_1_trap_handler(int tin)
{
	led_on(1);

	_out_uart('<');
	flush_stdout_trap();

	sprintf(g_tmp_output_buf, "[4>%i]\n", tin);
	_output_buf(g_tmp_output_buf, strlen(g_tmp_output_buf));

	sprintf(g_tmp_output_buf, "%08X %08X %08X %08X %08X %08X %08X\n",
			_mfcr( CPU_FCX ),
			_mfcr( CPU_LCX ),
			_mfcr( CPU_PCXI ),
			_mfcr( CPU_ISP ),
			_mfcr( CPU_PC ),
			_mfcr( CPU_DEADD ),
			_mfcr( CPU_DSTR )
	);
	_output_buf(g_tmp_output_buf, strlen(g_tmp_output_buf));

	while(1) {
		_nop();
	}

	switch (tin)
	{
	case 1:
		_debug(); /* PRIV -- Privileged Instruction  */
		break;

	case 2:
		_debug(); /* MPR -- MemProt: Read Access  */
		break;

	case 3:
		_debug(); /* MPW -- MemProt: Write Access  */
		break;

	case 4:
		_debug(); /* MPX -- MemProt: Execution Access  */
		break;

	case 5:
		_debug(); /* MPP -- MemProt: Peripheral Access  */
		break;

	case 6:
		_debug(); /* MPN -- MemProt: Null Address  */
		break;

	case 7:
		_debug(); /* GRPW -- Global Register Write Prot  */
		break;

	default:
		_debug();
		break;
	}
}

/* Instruction Errors  */

static void __class_2_trap_handler(int tin)
{
	led_on(2);

	_out_uart('<');
	flush_stdout_trap();
	_out_uart('2');
	flush_stdout_trap();
	_out_uart('>');
	flush_stdout_trap();
	_out_uart('[');
	flush_stdout_trap();
	_out_uart('0'+tin);
	flush_stdout_trap();
	_out_uart(']');
	flush_stdout_trap();

	switch (tin)
	{
	case 1:
		_debug(); /* IOPC -- Illegal Opcode  */
		break;

	case 2:
		_debug(); /* UOPC -- Unimplemented Opcode  */
		break;

	case 3:
		_debug(); /* OPD -- Invalid Operand Specification  */
		break;

	case 4:
		_debug(); /* ALN -- Data Address Alignment  */
		break;

	case 5:
		_debug(); /* MEM -- Invalid Local Memory Address  */
		break;

	default:
		_debug();
		break;
	}
}

/* Context Management  */

static void __class_3_trap_handler(int tin)
{
	_out_uart('3');
	flush_stdout_trap();
	_out_uart('_');
	flush_stdout_trap();
	_out_uart('0'+tin);
	flush_stdout_trap();

	switch (tin)
	{
	case 1:
		_debug(); /* FCD -- Free Context List Depletion  */
		break;

	case 2:
		_debug(); /* CDO -- Call Depth Overflow  */
		break;

	case 3:
		_debug(); /* CDU -- Call Depth Underflow  */
		break;

	case 4:
		_debug(); /* FCU -- Free Context List Underflow  */
		break;

	case 5:
		_debug(); /* CSU -- Call Stack Underflow  */
		break;

	case 6:
		_debug(); /* CTYP -- Context Type Error  */
		break;

	case 7:
		_debug(); /* NEST -- Nesting Error (RFE)  */
		break;

	default:
		_debug();
		break;
	}
}

/* System Bus and Peripheral Errors  */
static void __class_4_trap_handler(int tin) {
	led_on(0);
	led_on(1);

	_out_uart('<');
	flush_stdout_trap();

	sprintf(g_tmp_output_buf, "[4>%i]\n", tin);
	_output_buf(g_tmp_output_buf, strlen(g_tmp_output_buf));

	sprintf(g_tmp_output_buf, "%08X %08X %08X %08X %08X %08X %08X\n",
			_mfcr( CPU_FCX ),
			_mfcr( CPU_LCX ),
			_mfcr( CPU_PCXI ),
			_mfcr( CPU_ISP ),
			_mfcr( CPU_PC ),
			_mfcr( CPU_DEADD ),
			_mfcr( CPU_DSTR )
	);
	_output_buf(g_tmp_output_buf, strlen(g_tmp_output_buf));

	while(1) {
		_nop();
	}

	switch (tin) {
	case 1:
		_debug(); /* PSE -- Program Fetch Synchronous Error  */
		break;

	case 2:
//		A DSE trap is raised for the following conditions:
//		*An access outside the range of the DSPR (Scratch Range Error)
//		*An access to the lower half of segment C which cannot be translated into a global
//		address, i.e. from C1000000 to C7FFFFFF (Global Address Error)
//		*An error on the bus for an external accesses due to a load (Load Bus Error)
//		*An error from the bus during a cache refill (Cache Refill Error)
//		*An error during a load whilst in SIST mode (Load MSIST Error)
//		*An error generated by the overlay system during a load.
//		Whenever a DSE trap occurs, the DSTR (Data Synchronous Trap Register) and the
//		DEADD (Data Error Address Register) CSFRs are updated.
		_debug(); /* DSE -- Data Access Synchronous Error  */
		break;

	case 3:
		_debug(); /* DAE -- Data Access Asynchronous Error  */
		break;

	case 4:
		_debug(); /* CAO -- Coprocessor Trap Asynchronous Error  */
		break;

	case 5:
		_debug(); /* PIE -- Program Memory Integrity Error  */
		break;

	case 6:
		_debug(); /* DIE -- Data Memory Integrity Error  */
		break;

	case 7:
		_debug(); /* TAE -- Temporal Asynchronous Error  */
		break;

	default:
		_debug();
		break;
	}
}

/* Assertion Traps  */

static void __class_5_trap_handler(int tin)
{
	led_on(1);
	led_on(2);

	_out_uart('<');
	flush_stdout_trap();
	_out_uart('5');
	flush_stdout_trap();
	_out_uart('>');
	flush_stdout_trap();
	_out_uart('[');
	flush_stdout_trap();
	_out_uart('0'+tin);
	flush_stdout_trap();
	_out_uart(']');
	flush_stdout_trap();

	switch (tin)
	{
	case 1:
		_debug(); /* OVF -- Arithmetic Overflow  */
		break;

	case 2:
		_debug(); /* SOVF -- Sticky Arithmetic Overflow  */
		break;

	default:
		_debug();
		break;
	}
}

/* Non-maskable Interrupt  */

static void __class_7_trap_handler(int tin)
{
	led_on(2);
	led_on(3);

	_out_uart('<');
	flush_stdout_trap();
	_out_uart('7');
	flush_stdout_trap();
	_out_uart('>');
	flush_stdout_trap();
	_out_uart('[');
	flush_stdout_trap();
	_out_uart('0'+tin);
	flush_stdout_trap();
	_out_uart(']');
	flush_stdout_trap();

	(void)tin;
	_debug(); /* NMI -- Non-maskable Interrupt  */
}

/* The default handler for interrupts.  */

static void default_isr(int arg) {
	/* Just ignore this interrupt.  */
	UNUSED(arg);
}

/* Install INTHANDLER for interrupt INTNO and remember ARG for later use.  */

int _install_int_handler(int prio, void (*inthandler)(int), int arg)
{
	if ((prio < 0) || (prio >= MAX_INTRS))
		return 0;

	Cdisptab[prio].hnd_handler = inthandler;
	Cdisptab[prio].hnd_arg = arg;

	return 1;
}

/* This initializes the C interrupt interface.  */

void _init_vectab(void);

extern int TriCore_trap_table[];
extern int TriCore_int_table[];

/* System Call #tin  */
extern void prvTrapYield( int iTrapIdentification );

void _init_vectab(void)
{
	/* Set BTV and BIV registers.  */
	unlock_wdtcon();
	_mtcr(CPU_BTV, (uint32_t)TriCore_trap_table);
	_mtcr(CPU_BIV, (uint32_t)TriCore_int_table);
	lock_wdtcon();

	/* Initialize the trap handlers.  */
	Tdisptab[0] = __class_0_trap_handler;
	Tdisptab[1] = __class_1_trap_handler;
	Tdisptab[2] = __class_2_trap_handler;
	Tdisptab[3] = __class_3_trap_handler;
	Tdisptab[4] = __class_4_trap_handler;
	Tdisptab[5] = __class_5_trap_handler;
	Tdisptab[6] = prvTrapYield;
	Tdisptab[7] = __class_7_trap_handler;

	/* Initialize the interrupt handlers.  */
	for (uint16_t vecno = 0; vecno < MAX_INTRS; ++vecno)
	{
		Cdisptab[vecno].hnd_handler = default_isr;
		Cdisptab[vecno].hnd_arg = vecno;
	}
}
