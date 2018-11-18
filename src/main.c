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

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxEth_reg.h)
#include TC_INCLUDE(TCPATH/IfxEth_bf.h)
#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxInt_reg.h)
#include TC_INCLUDE(TCPATH/IfxInt_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)

#include "core_tc23x.h"
#include "system_tc2x.h"
#include "cint_trap_tc23x.h"
#include "interrupts_tc23x.h"
#include "led.h"
#include "uart_int.h"
#include "asm_prototype.h"
#include "dts.h"
#include "timer.h"
#include "gpsr.h"
#include "scu_eru.h"
#include "data_flash.h"

void enable_performance_cnt(void)
{
	unlock_wdtcon();
	{
		Ifx_CPU_CCTRL tmpCCTRL;
		tmpCCTRL.U = _mfcr(CPU_CCTRL);

		//Instruction Cache Hit Count
		//		tmpCCTRL.bits.M1 = 2;
		//Data Cache Hit Count.
		tmpCCTRL.B.M1 = 3;
		//Instruction Cache Miss Count

		//		tmpCCTRL.bits.M2 = 1;
		//Data Cache Clean Miss Count
		tmpCCTRL.B.M2 = 3;

		//Data Cache Dirty Miss Count
		tmpCCTRL.B.M3 = 3;

		//Normal Mode
		tmpCCTRL.B.CM = 0;
		//Task Mode
		//		tmpCCTRL.bits.CM = 1;

		tmpCCTRL.B.CE = 1;

		_mtcr(CPU_CCTRL, tmpCCTRL.U);
	}
	lock_wdtcon();
}

int core0_main(int argc, char** argv)
{
	volatile bool g_regular_task_flag;

	system_clk_config_200_100();

	/* activate interrupt system */
	InterruptInit();

	SYSTEM_Init();
	SYSTEM_EnaDisCache(1);

	/* initialise STM CMP 0 at SYS_TICK_HZ rate */
	stm_init(0, SYS_TICK_HZ);
	stm_init(1, 10*SYS_TICK_HZ);

	uart_init(BAUDRATE);
	led_init();

	config_dts();
//	config_gpsr();
//	config_eru();
//	config_dflash();

	enable_performance_cnt();

	printf("%s %s\n", _NEWLIB_VERSION, __func__);

	printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,Int:%u M,CE:%d\n",
			__TRICORE_NAME__,
			__TRICORE_CORE__,
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			system_GetPllClock()/1000000,
			system_GetIntClock()/1000000,
			SYSTEM_IsCacheEnabled());

	printf("Core\t%08X\t:%08X\n", CPU_CORE_ID, _mfcr(CPU_CORE_ID));
	printf("CUS\t%08X\t:%08X\n", CPU_CUS_ID, _mfcr(CPU_CUS_ID));
	printf("CUS\t%08X\t:%08X\n", CPU_CUS_ID, _mfcr(CPU_CUS_ID));

	flush_stdout();

	printf("\nTest PLL\n");
	flush_stdout();

	flush_stdout();
	_syscall(0);
	_syscall(1);
	_syscall(255);

	g_regular_task_flag = true;

	uint8_t test_trig_cnt = 0;
	while(1)
	{
#define	DELAY_TICK	(10*SYS_TICK_HZ)
		if(0==HAL_GetTick()%(DELAY_TICK))
		{
			g_regular_task_flag = true;
		}

		uint32_t test_trigger_cnt = HAL_GetTick()/(DELAY_TICK);

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

			if(0==test_trigger_cnt%4)
			{
				led_on(0);
				led_off(1);
				led_off(2);
			}
			else if(1==test_trigger_cnt%4)
			{
				led_off(0);
				led_on(1);
				led_off(2);
			}
			else if(2==test_trigger_cnt%4)
			{
				led_off(0);
				led_off(1);
				led_on(2);
			}
			else
			{
				led_toggle(3);
			}

			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,Int:%u M,CE:%d\n",
					__TRICORE_NAME__,
					__TRICORE_CORE__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					SYSTEM_GetStmClock()/1000000,
					system_GetPllClock()/1000000,
					system_GetIntClock()/1000000,
					SYSTEM_IsCacheEnabled());
			flush_stdout();

			start_dts_measure();

			printf("%u\n", HAL_GetTick());
			printf("%u\n", HAL_GetRunTimeTick());
			flush_stdout();

			printf("OSCON:%08X PCON0:%08X PCON1:%08X PCON2:%08X CCON0:%08X CCON1:%08X CCON2:%08X CCON3:%08X CCON4:%08X CCON5%08X CCON6%08X CCON9:%08X\n",
					MODULE_SCU.OSCCON.U,
					MODULE_SCU.PLLCON0.U,
					MODULE_SCU.PLLCON1.U,
					MODULE_SCU.PLLCON2.U,
					MODULE_SCU.CCUCON0.U,
					MODULE_SCU.CCUCON1.U,
					MODULE_SCU.CCUCON2.U,
					MODULE_SCU.CCUCON3.U,
					MODULE_SCU.CCUCON4.U,
					MODULE_SCU.CCUCON5.U,
					MODULE_SCU.CCUCON6.U,
					MODULE_SCU.CCUCON9.U
			);
			flush_stdout();

			test_trig_cnt = test_trigger_gpsr(test_trig_cnt);

		}

		test_proc_dts();
//		test_proc_eru();
//		test_proc_gpsr();
	}

	return EXIT_SUCCESS;
}

