/*
 * dts.c
 *
 *  Created on: Nov 17, 2018
 *      Author: zzr
 */

#include "dts.h"

#include "uart_int.h"

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

static volatile bool g_dts_flag;

extern TaskHandle_t g_info_task_handler;

bool IfxDts_isReady(void)
{
	return SCU_DTSSTAT.B.RDY == 1 ? true : false;
}

void dts_isr(uint32_t para)
{
//	g_dts_flag = true;
	xTaskNotifyFromISR( g_info_task_handler, 3, eSetValueWithOverwrite, NULL);
}

void config_dts(void)
{
	//	printf("SCU_DTSCON\t%08X\t:%08X\n", &SCU_DTSCON, SCU_DTSCON);
	//	printf("SCU_DTSSTAT\t%08X\t:%08X\n", &SCU_DTSSTAT, SCU_DTSSTAT);
	//	printf("SCU_DTSLIM\t%08X\t:%08X\n", &SCU_DTSLIM, SCU_DTSLIM);
	//	flush_stdout();

	SCU_DTSCON.B.PWD = 0;
	SCU_DTSCON.B.START = 1;

	if(IfxDts_isReady())
	{
		/* one dummy read to a SCU register which ensures that the BUSY flag is synchronized
		 * into the status register before IfxDts_Dts_isBusy() is called */
		_nop();
	}

	InterruptInstall(SRC_ID_SCUDTS, dts_isr, DTS_ISR_PRIO, 0);
}

int16_t read_dts(void)
{
	return SCU_DTSSTAT.B.RESULT;
}

float read_dts_celsius(void)
{
	int16_t raw = read_dts();
	return (raw - 607)/2.13;
}

void start_dts_measure(void)
{
	SCU_DTSCON.B.START = 1;
}

void test_proc_dts(void)
{
	if(g_dts_flag)
	{
		g_dts_flag = false;
		printf("DTS Triggered %.3f P15.4:%u\n",
				read_dts_celsius(),
				MODULE_P15.IN.B.P4);
		flush_stdout();
	}
}

