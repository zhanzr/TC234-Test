/*
 * scu_eru.c
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#include "scu_eru.h"

#include "uart_int.h"

static volatile bool g_ers_flag[4];

void ers0_isr(uint32_t var)
{
	UNUSED(var);
	MODULE_SCU.FMR.B.FC0 = 1;
	g_ers_flag[0] = true;
}

void ers1_isr(uint32_t var)
{
	UNUSED(var);
	MODULE_SCU.FMR.B.FC1 = 1;
	g_ers_flag[1] = true;
}

void ers2_isr(uint32_t var)
{
	UNUSED(var);
	MODULE_SCU.FMR.B.FC2 = 1;
	g_ers_flag[2] = true;
}

void ers3_isr(uint32_t var)
{
	UNUSED(var);
	MODULE_SCU.FMR.B.FC3 = 1;
	g_ers_flag[3] = true;
}

void config_eru(void)
{
	//ERS 0, 2, 3, 6, 7
	//	ERS0:
	//		P15.4		->X102.P33
	//
	//	ERS1:
	//		P14.3
	//
	//	ERS2:
	//		P10.2
	//		P02.1		->X103.P14
	//		P00.4		->X103.P26
	//
	//	ERS3:
	//		P10.3
	//		P14.1
	//		P02.0		->X103.P13
	//
	//	ERS4:
	//		P33.7
	//		P15.5
	//
	//	ERS5:
	//		P15.8
	//
	//	ERS6:
	//		P20.0
	//		P33.11		-> X102.P26
	//		P11.10
	//
	//	ERS7:
	//		P20.9		-> X102.P37
	//		P15.1

	//ERS Channel 0
	MODULE_P15.IOCR4.B.PC4 = IN_PULLUP;

	MODULE_SCU.EICR[0].B.EXIS0 = 0;
	MODULE_SCU.EICR[0].B.FEN0 = 1;
	MODULE_SCU.EICR[0].B.REN0 = 0;
	MODULE_SCU.EICR[0].B.LDEN0 = 0;
	MODULE_SCU.EICR[0].B.EIEN0 = 1;
	MODULE_SCU.EICR[0].B.INP0 = 0;

	MODULE_SCU.IGCR[0].B.IGP0 = 2;
	MODULE_SCU.IGCR[0].B.GEEN0 = 1;

	//ERS2 Channel 1
	MODULE_P02.IOCR0.B.PC1 = IN_PULLUP;

	MODULE_SCU.EICR[1].B.EXIS0 = 1;
	MODULE_SCU.EICR[1].B.FEN0 = 1;
	MODULE_SCU.EICR[1].B.REN0 = 0;
	MODULE_SCU.EICR[1].B.LDEN0 = 0;
	MODULE_SCU.EICR[1].B.EIEN0 = 1;
	MODULE_SCU.EICR[1].B.INP0 = 0;

	MODULE_SCU.IGCR[1].B.IGP0 = 2;
	MODULE_SCU.IGCR[1].B.GEEN0 = 1;

	//ERS3 Channel 2
	MODULE_P02.IOCR0.B.PC0 = IN_PULLUP;

	MODULE_SCU.EICR[1].B.EXIS1 = 2;
	MODULE_SCU.EICR[1].B.FEN1 = 1;
	MODULE_SCU.EICR[1].B.REN1 = 0;
	MODULE_SCU.EICR[1].B.LDEN1 = 0;
	MODULE_SCU.EICR[1].B.EIEN1 = 1;
	MODULE_SCU.EICR[1].B.INP1 = 0;

	MODULE_SCU.IGCR[1].B.IGP1 = 2;
	MODULE_SCU.IGCR[1].B.GEEN1 = 1;

	//ERS6 Channel 2
	MODULE_P33.IOCR8.B.PC11 = IN_PULLUP;

	MODULE_SCU.EICR[3].B.EXIS0 = 2;
	MODULE_SCU.EICR[3].B.FEN0 = 1;
	MODULE_SCU.EICR[3].B.REN0 = 0;
	MODULE_SCU.EICR[3].B.LDEN0 = 0;
	MODULE_SCU.EICR[3].B.EIEN0 = 1;
	MODULE_SCU.EICR[3].B.INP0 = 0;

	MODULE_SCU.IGCR[3].B.IGP0 = 2;
	MODULE_SCU.IGCR[3].B.GEEN0 = 1;

	//ERS7 Channel 0
	MODULE_P20.IOCR8.B.PC9 = IN_PULLUP;

	MODULE_SCU.EICR[3].B.EXIS1 = 0;
	MODULE_SCU.EICR[3].B.FEN1 = 1;
	MODULE_SCU.EICR[3].B.REN1 = 0;
	MODULE_SCU.EICR[3].B.LDEN1 = 0;
	MODULE_SCU.EICR[3].B.EIEN1 = 1;
	MODULE_SCU.EICR[3].B.INP1 = 0;

	MODULE_SCU.IGCR[3].B.IGP1 = 2;
	MODULE_SCU.IGCR[3].B.GEEN1 = 1;

	InterruptInstall(SRC_ID_SCUERU0, ers0_isr, ERS0_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU1, ers1_isr, ERS1_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU2, ers2_isr, ERS2_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU3, ers3_isr, ERS3_ISR_PRIO, 0);
}

void test_proc_eru(void)
{
	if(g_ers_flag[0])
	{
		g_ers_flag[0] = false;
		//			printf("External Interrupt Channel 0 Triggered\n");
		flush_stdout();
	}

	if(g_ers_flag[1])
	{
		g_ers_flag[1] = false;
		//			printf("External Interrupt Channel 1 Triggered\n");
		flush_stdout();
	}

	if(g_ers_flag[2])
	{
		g_ers_flag[2] = false;
		//			printf("External Interrupt Channel 2 Triggered\n");
		flush_stdout();
	}

	if(g_ers_flag[3])
	{
		g_ers_flag[3] = false;
		//			printf("External Interrupt Channel 3 Triggered\n");
		flush_stdout();
	}
}
