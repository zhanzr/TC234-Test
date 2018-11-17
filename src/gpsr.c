/*
 * gpsr.c
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#include "gpsr.h"

#include "uart_int.h"

static volatile bool g_gpsr_flag[4];

void gpsr0_isr(uint32_t var)
{
	UNUSED(var);
	SRC_GPSR00.B.SRR = 0;
	g_gpsr_flag[0] = true;
}

void gpsr1_isr(uint32_t var)
{
	UNUSED(var);
	SRC_GPSR01.B.SRR = 0;
	g_gpsr_flag[1] = true;
}

void gpsr2_isr(uint32_t var)
{
	UNUSED(var);
	SRC_GPSR02.B.SRR = 0;
	g_gpsr_flag[2] = true;
}

void gpsr3_isr(uint32_t var)
{
	UNUSED(var);
	SRC_GPSR03.B.SRR = 0;
	g_gpsr_flag[3] = true;
}

void config_gpsr(void)
{
	SRC_GPSR00.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR00, gpsr0_isr, GPSR0_ISR_PRIO, 0);

	SRC_GPSR01.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR01, gpsr1_isr, GPSR1_ISR_PRIO, 1);

	SRC_GPSR02.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR02, gpsr2_isr, GPSR2_ISR_PRIO, 2);

	SRC_GPSR03.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR03, gpsr3_isr, GPSR3_ISR_PRIO, 3);
}

//Test Function, Trigger and parser

uint8_t test_trigger_gpsr(uint8_t t)
{
	switch(t)
	{
	case 0:
		INT_SRB0.B.TRIG0 = 1;
		//				SRC_GPSR00.B.SETR = 1;
		break;

	case 1:
		INT_SRB0.B.TRIG1 = 1;
		//				SRC_GPSR01.B.SETR = 1;
		break;

	case 2:
		//				INT_SRB0.B.TRIG2 = 1;
		SRC_GPSR02.B.SETR = 1;
		break;

	case 3:
		//				INT_SRB0.B.TRIG3 = 1;
		SRC_GPSR03.B.SETR = 1;
		break;

	default:
		break;
	}
	t = (t+1)%4;

	return t;
}

void test_proc_gpsr(void)
{
	if(g_gpsr_flag[0])
	{
		g_gpsr_flag[0] = false;
		printf("GPSR 0 Triggered\n");
		flush_stdout();
	}

	if(g_gpsr_flag[1])
	{
		g_gpsr_flag[1] = false;
		printf("GPSR 1 Triggered\n");
		flush_stdout();
	}

	if(g_gpsr_flag[2])
	{
		g_gpsr_flag[2] = false;
		printf("GPSR 2 Triggered\n");
		flush_stdout();
	}

	if(g_gpsr_flag[3])
	{
		g_gpsr_flag[3] = false;
		printf("GPSR 3 Triggered\n");
		flush_stdout();
	}
}

