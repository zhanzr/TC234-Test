/*
 * data_flash.c
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#include "data_flash.h"

#include "uart_int.h"

void config_dflash(void)
{
	printf("BMHD Size:%u, ADDR:%08X %08X %08X %08X\n",
			sizeof(BMHD_t),
			BMHD_ADDR_0,
			P_BMHD_1,
			P_BMHD_2,
			P_BMHD_3
	);
	flush_stdout();

	printf("BMHD0, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
			(uint32_t)P_BMHD_0->startAddress,
			P_BMHD_0->bmIndex,
			P_BMHD_0->bmhdID,
			P_BMHD_0->chkStart,
			P_BMHD_0->chkEnd,
			P_BMHD_0->crcRange,
			P_BMHD_0->invCrcRange,
			P_BMHD_0->crcHead,
			P_BMHD_0->invCrcHead);
	flush_stdout();

//	printf("MODULE_PMU0.ID\t%08X\t:%08X\n", (uint32_t)&MODULE_PMU0.ID, MODULE_PMU0.ID.U);
//	printf("MODULE_FLASH0.ID\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.ID.U);
	printf("MODULE_FLASH0.ACCEN0\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ACCEN0, MODULE_FLASH0.ACCEN0.U);
//	printf("MODULE_FLASH0.ACCEN1\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ACCEN1, MODULE_FLASH0.ACCEN1.U);
	printf("MODULE_FLASH0.FSR\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.FSR, MODULE_FLASH0.FSR.U);
	printf("MODULE_FLASH0.FCON\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.FCON, MODULE_FLASH0.FCON.U);
	printf("MODULE_FLASH0.CBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.CBAB[0].CFG, MODULE_FLASH0.CBAB[0].CFG.U);
	printf("MODULE_FLASH0.CBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.CBAB[0].STAT, MODULE_FLASH0.CBAB[0].STAT.U);
	printf("MODULE_FLASH0.CBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.CBAB[0].TOP, MODULE_FLASH0.CBAB[0].TOP.U);
	printf("MODULE_FLASH0.COMM0\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.COMM0, MODULE_FLASH0.COMM0.U);
	printf("MODULE_FLASH0.COMM1\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.COMM1, MODULE_FLASH0.COMM1.U);
	printf("MODULE_FLASH0.COMM2\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.COMM2, MODULE_FLASH0.COMM2.U);
	flush_stdout();
	printf("MODULE_FLASH0.FPRO\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.FPRO, MODULE_FLASH0.FPRO.U);
	printf("MODULE_FLASH0.PROCONP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCONP[0], MODULE_FLASH0.PROCONP[0].U);
	printf("MODULE_FLASH0.PROCOND\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCOND, MODULE_FLASH0.PROCOND.U);
	printf("MODULE_FLASH0.PROCONOTP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCONOTP[0], MODULE_FLASH0.PROCONOTP[0].U);
	printf("MODULE_FLASH0.PROCONWOP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCONWOP[0], MODULE_FLASH0.PROCONWOP[0].U);
	printf("MODULE_FLASH0.PROCONDBG\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCONDBG, MODULE_FLASH0.PROCONDBG.U);
	printf("MODULE_FLASH0.PROCONHSMCOTP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.PROCONHSMCOTP, MODULE_FLASH0.PROCONHSMCOTP.U);
//	printf("MODULE_FLASH0.ECCRD\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.ECCRD.U);
//	printf("MODULE_FLASH0.ECCRP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.ECCRP[0].U);
//	printf("MODULE_FLASH0.ECCW\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.ECCW.U);
//	printf("MODULE_FLASH0.HSMFCON\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMFCON.U);
//	printf("MODULE_FLASH0.HSMFSR\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMFSR.U);
//	printf("MODULE_FLASH0.HSMMARD\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMMARD.U);
//	printf("MODULE_FLASH0.HSMRRAD\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMRRAD.U);
//	printf("MODULE_FLASH0.HSMRRCT\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMRRCT.U);
//	printf("MODULE_FLASH0.HSMRRD0\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMRRD0.U);
//	printf("MODULE_FLASH0.HSMRRD1\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.HSMRRD1.U);
	flush_stdout();
//	printf("MODULE_FLASH0.MARD\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.MARD.U);
//	printf("MODULE_FLASH0.MARP\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.MARP.U);
//	printf("MODULE_FLASH0.PROCONHSM\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.PROCONHSM.U);
//	printf("MODULE_FLASH0.RDBCFG\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RDBCFG[0].CFG0.U);
//	flush_stdout();
//	printf("MODULE_FLASH0.RDBCFG\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RDBCFG[0].CFG1.U);
//	printf("MODULE_FLASH0.RDBCFG\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RDBCFG[0].CFG2.U);
//	printf("MODULE_FLASH0.RRAD\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RRAD.U);
//	printf("MODULE_FLASH0.RRCT\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RRCT.U);
//	printf("MODULE_FLASH0.RRD0\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RRD0.U);
//	printf("MODULE_FLASH0.RRD1\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.RRD1.U);
//	printf("MODULE_FLASH0.UBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.UBAB[0].CFG.U);
//	printf("MODULE_FLASH0.UBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.UBAB[0].STAT.U);
//	printf("MODULE_FLASH0.UBAB\t%08X\t:%08X\n", (uint32_t)&MODULE_FLASH0.ID, MODULE_FLASH0.UBAB[0].TOP.U);
//	flush_stdout();
//
//	extern void __PMU_DFLASH0_BEGIN(void);
//	extern void __PMU_DFLASH0_SIZE(void);
//	printf("DF0:%08X %08X\n", (uint32_t)__PMU_DFLASH0_BEGIN, (uint32_t)__PMU_DFLASH0_SIZE);
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)((uint32_t)__PMU_DFLASH0_BEGIN+i*4)));
//	}
//	printf("\n");
//	flush_stdout();


	//Read UCB
//#define	UCB_ADDR	(0xAF100000)
//#define	UCB_SIZE	(0x4000)
//	printf("UCB:%08X %08X\n", (uint32_t)UCB_ADDR, (uint32_t)UCB_SIZE);
//	for(uint32_t i=0; i<UCB_SIZE/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)((uint32_t)UCB_ADDR+i*4)));
//	}
//	printf("\n");
//	flush_stdout();

//	printf("BMHD0:%08X\n", BMHD_ADDR_0);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_0+i*4)));
//		flush_stdout();
//	}

//	unlock_wdtcon();
//	uint32_t tmpU32 = *((uint32_t*)(BMHD_ADDR_1));
//	lock_wdtcon();
//	printf("%08X\n", tmpU32);
//	flush_stdout();
//
//	printf("BMHD1:%08X\n", BMHD_ADDR_1);
//	flush_stdout();
//
//	unlock_wdtcon();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_1+i*4)));
//	}
//	lock_wdtcon();
//	flush_stdout();

//	printf("BMHD2:%08X\n", BMHD_ADDR_2);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_2+i*4)));
//		flush_stdout();
//	}
//
//	printf("BMHD3:%08X\n", BMHD_ADDR_3);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_3+i*4)));
//		flush_stdout();
//	}

//	unlock_wdtcon();
//	printf("BMHD1, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_1->startAddress,
//			P_BMHD_1->bmIndex,
//			P_BMHD_1->bmhdID,
//			P_BMHD_1->chkStart,
//			P_BMHD_1->chkEnd,
//			P_BMHD_1->crcRange,
//			P_BMHD_1->invCrcRange,
//			P_BMHD_1->crcHead,
//			P_BMHD_1->invCrcHead);
//	flush_stdout();
//	lock_wdtcon();

//	printf("BMHD2, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_2->startAddress,
//			P_BMHD_2->bmIndex,
//			P_BMHD_2->bmhdID,
//			P_BMHD_2->chkStart,
//			P_BMHD_2->chkEnd,
//			P_BMHD_2->crcRange,
//			P_BMHD_2->invCrcRange,
//			P_BMHD_2->crcHead,
//			P_BMHD_2->invCrcHead);
//	flush_stdout();
//
//	printf("P_BMHD_3, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_3->startAddress,
//			P_BMHD_3->bmIndex,
//			P_BMHD_3->bmhdID,
//			P_BMHD_3->chkStart,
//			P_BMHD_3->chkEnd,
//			P_BMHD_3->crcRange,
//			P_BMHD_3->invCrcRange,
//			P_BMHD_3->crcHead,
//			P_BMHD_3->invCrcHead);
//	flush_stdout();


	extern void __PMI_PSPR_BEGIN(void);
	extern void __PMI_PSPR_SIZE(void);
	printf("PSRR:%08X %08X\n", (uint32_t)__PMI_PSPR_BEGIN, (uint32_t)__PMI_PSPR_SIZE);
	for(uint32_t i=0; i<0x20/4; ++i)
	{
		printf("%08X ", *((uint32_t*)((uint32_t)__PMI_PSPR_BEGIN+i*4)));
	}
	printf("\n");
	flush_stdout();

	//	printf("ETH_ID\t%08X\t:%08X\n", &ETH_ID, ETH_ID);
//	printf("SCU_ID\t%08X\t:%08X\n", &SCU_ID, SCU_ID);
//	printf("SCU_MANID\t%08X\t:%08X\n", &SCU_MANID, SCU_MANID);
//	printf("SCU_CHIPID\t%08X\t:%08X\n", &SCU_CHIPID, SCU_CHIPID);
//
//	printf("INT_ID\t%08X\t:%08X\n", &INT_ID, INT_ID);
//	printf("INT_SRB0\t%08X\t:%08X\n", &INT_SRB0, INT_SRB0);
//	printf("SRC_GPSR00\t%08X\t:%08X\n", &SRC_GPSR00, SRC_GPSR00);
//	printf("SRC_GPSR01\t%08X\t:%08X\n", &SRC_GPSR01, SRC_GPSR01);
//	printf("SRC_GPSR02\t%08X\t:%08X\n", &SRC_GPSR02, SRC_GPSR02);
//	printf("SRC_GPSR03\t%08X\t:%08X\n", &SRC_GPSR03, SRC_GPSR03);
}

void test_proc_dflash(void)
{
	//	printf("CPUID\t%08X\t:%08X\n", CPU_CPU_ID, _mfcr(CPU_CPU_ID));
		//	printf("CCTRL\t%08X\t:%08X\n", CPU_CCTRL, _mfcr(CPU_CCTRL));
		//	printf("CCNT\t%08X\t:%08X\n", CPU_CCNT, _mfcr(CPU_CCNT));
		//	printf("ICNT\t%08X\t:%08X\n", CPU_ICNT, _mfcr(CPU_ICNT));
		//	printf("M1CNT\t%08X\t:%08X\n", CPU_M1CNT, _mfcr(CPU_M1CNT));
		//	printf("M2CNT\t%08X\t:%08X\n", CPU_M2CNT, _mfcr(CPU_M2CNT));
		//	printf("M3CNT\t%08X\t:%08X\n", CPU_M3CNT, _mfcr(CPU_M3CNT));

	extern void __USTACK_BEGIN(void);
	printf("__USTACK_BEGIN\t:%08X\n", (uint32_t)__USTACK_BEGIN);
	extern void __USTACK(void);
	printf("__USTACK\t:%08X\n", (uint32_t)__USTACK);
	extern void __USTACK_SIZE(void);
	printf("__USTACK_SIZE\t:%08X\n", (uint32_t)__USTACK_SIZE);
	extern void __USTACK_END(void);
	printf("__USTACK_END\t:%08X\n", (uint32_t)__USTACK_END);
	flush_stdout();

	extern void __ISTACK_BEGIN(void);
	printf("__ISTACK_BEGIN\t:%08X\n", (uint32_t)__ISTACK_BEGIN);
	extern void __ISTACK(void);
	printf("__ISTACK\t:%08X\n", (uint32_t)__ISTACK);
	extern void __ISTACK_SIZE(void);
	printf("__ISTACK_SIZE\t:%08X\n", (uint32_t)__ISTACK_SIZE);
	extern void __ISTACK_END(void);
	printf("__ISTACK_END\t:%08X\n", (uint32_t)__ISTACK_END);
	flush_stdout();

	extern void __HEAP_BEGIN(void);
	printf("__HEAP_BEGIN\t:%08X\n", (uint32_t)__HEAP_BEGIN);
	extern void __HEAP(void);
	printf("__HEAP\t:%08X\n", (uint32_t)__HEAP);
	extern void __HEAP_SIZE(void);
	printf("__HEAP_SIZE\t:%08X\n", (uint32_t)__HEAP_SIZE);
	extern void __HEAP_END(void);
	printf("__HEAP_END\t:%08X\n", (uint32_t)__HEAP_END);
	flush_stdout();

	extern void __CSA_BEGIN(void);
	printf("__CSA_BEGIN\t:%08X\n", (uint32_t)__CSA_BEGIN);
	extern void __CSA(void);
	printf("__CSA\t:%08X\n", (uint32_t)__CSA);
	extern void __CSA_SIZE(void);
	printf("__CSA_SIZE\t:%08X\n", (uint32_t)__CSA_SIZE);
	extern void __CSA_END(void);
	printf("__CSA_END\t:%08X\n", (uint32_t)__CSA_END);
	flush_stdout();

	extern void _SMALL_DATA_(void);
	printf("_SMALL_DATA_\t:%08X\n", (uint32_t)_SMALL_DATA_);
	extern void _SMALL_DATA2_(void);
	printf("_SMALL_DATA2_\t:%08X\n", (uint32_t)_SMALL_DATA2_);

	extern void first_trap_table(void);
	printf("first_trap_table\t:%08X\n", (uint32_t)first_trap_table);

	printf("BIV\t%08X\t:%08X\n", CPU_BIV, _mfcr(CPU_BIV));
	extern void TriCore_int_table(void);
	printf("TriCore_int_table\t:%08X\n", (uint32_t)TriCore_int_table);
	flush_stdout();

//	extern void __interrupt_1(void);
//	printf("__interrupt_1\t:%08X\n", (uint32_t)__interrupt_1);
//	extern void ___interrupt_1(void);
//	printf("___interrupt_1\t:%08X\n", (uint32_t)___interrupt_1);
//	extern void __interrupt_2(void);
//	printf("__interrupt_2\t:%08X\n", (uint32_t)__interrupt_2);
//	flush_stdout();

//	extern Hnd_arg Cdisptab[MAX_INTRS];
//	printf("Soft Interrupt vector table %08X:%u * %u = %u\n",
//			(uint32_t)Cdisptab,
//			sizeof(Cdisptab[0]),
//			MAX_INTRS,
//			sizeof(Cdisptab));
//	flush_stdout();
//
//	printf("BTV\t%08X\t:%08X\n", CPU_BTV, _mfcr(CPU_BTV));
//	extern void TriCore_trap_table(void);
//	printf("TriCore_trap_table\t:%08X\n", (uint32_t)TriCore_trap_table);

//	extern void __trap_0(void);
//	printf("__trap_0\t:%08X\n", (uint32_t)__trap_0);
//	extern void __trap_1(void);
//	printf("__trap_1\t:%08X\n", (uint32_t)__trap_1);
//	extern void __trap_6(void);
//	printf("__trap_6\t:%08X\n", (uint32_t)__trap_6);
//	extern void ___trap_6(void);
//	printf("___trap_6\t:%08X\n", (uint32_t)___trap_6);
//	flush_stdout();

//	extern void (*Tdisptab[MAX_TRAPS]) (int tin);
//	printf("Soft Trap vector table %08X:%u * %u = %u\n",
//			(uint32_t)Tdisptab,
//			sizeof(Tdisptab[0]),
//			MAX_TRAPS,
//			sizeof(Tdisptab));
//	flush_stdout();
}

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

void test_perf_cnt_proc(void)
{

}
