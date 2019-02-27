/*====================================================================
 * Project:  Board Support Package (BSP)
 * Function: Extended system control API implementation for TC23x
 *           (based on PLS original sources)
 *           (adapted by HighTec for correct frequency up stepping)
 *
 * Copyright HighTec EDV-Systeme GmbH 1982-2016
 *====================================================================*/

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>

#include "tc_inc_path_tc23x.h"
#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxPort_reg.h)
#include TC_INCLUDE(TCPATH/IfxQspi_reg.h)

#include "bspconfig_tc23x.h"
#include "system_tc2x.h"
#include "interrupts_tc23x.h"
#include "asm_prototype.h"

/* for serving A-step and B-step (+ newer) TLF devices: use both commands for err pin monitor */
/* Workaround for TLF35584 A-Step Bug on AppKit-TC2x4 and AppKit-TC2x7 */
static void disable_external_watchdog(void)
{
	/* check that this disabling has not been already done (e.g. by the debugger) */
	if (QSPI2_GLOBALCON.B.EN){
		/* don't do it again */
		return;
	}

	/* initialise QSPI2 interface */
	unlock_wdtcon();				/* remove ENDINIT protection */
	/* activate module, disable sleep mode */
	QSPI2_CLC.B.EDIS = 1;
	QSPI2_CLC.B.DISR = 0;
	QSPI2_CLC.B.DISS = 0;
	_dsync();
	_isync();

	P15_PDR0.U = 0x00000000;		/* fast speed (all pins) */
	P14_PDR0.U = 0x00000000;		/* fast speed (all pins) */

	QSPI2_PISEL.B.SCIS = 0;
	QSPI2_PISEL.B.SLSIS = 0;
	QSPI2_PISEL.B.SRIS = 0;
	/* MRIS=1 ==> use MRST2B pin */
	QSPI2_PISEL.B.MRIS = 1;
	lock_wdtcon();					/* re-enable ENDINIT protection */

	/* configure port pins */
	P14_IOCR0.B.PC2 = OUT_PPALT3;			/* SLSO21 */
	P15_IOCR0.B.PC3 = OUT_PPALT3;			/* SCLK2 */
	P15_IOCR4.B.PC5 = OUT_PPALT3;			/* MTSR2 */
	P15_IOCR4.B.PC7 = IN_PULLUP;			/* MRST2B */

	/* program QSPI2 parameters */
	QSPI2_GLOBALCON.B.AREN = 0;
	QSPI2_GLOBALCON.B.DEL0 = 0;
	QSPI2_GLOBALCON.B.EN = 0;
	QSPI2_GLOBALCON.B.EXPECT = 15;
	QSPI2_GLOBALCON.B.LB = 0;
	QSPI2_GLOBALCON.B.MS = 0;
	QSPI2_GLOBALCON.B.RESETS = 0;
	QSPI2_GLOBALCON.B.SI = 0;
	QSPI2_GLOBALCON.B.SRF = 0;
	QSPI2_GLOBALCON.B.STIP = 0;
	QSPI2_GLOBALCON.B.STROBE = 0;
	QSPI2_GLOBALCON.B.TQ = 4;

	QSPI2_GLOBALCON1.B.RXFM = 1;
	QSPI2_GLOBALCON1.B.TXFM = 1;
	QSPI2_GLOBALCON1.B.RXFIFOINT = 0;
	QSPI2_GLOBALCON1.B.TXFIFOINT = 0;
	QSPI2_GLOBALCON1.B.ERRORENS = 0;
	QSPI2_GLOBALCON1.B.PT1 = 0;
	QSPI2_GLOBALCON1.B.PT1EN = 0;
	QSPI2_GLOBALCON1.B.PT2 = 0;
	QSPI2_GLOBALCON1.B.PT2EN = 0;
	QSPI2_GLOBALCON1.B.TXEN = 0;
	QSPI2_GLOBALCON1.B.RXEN = 0;
	QSPI2_GLOBALCON1.B.USREN = 0;

	/* enable SLSO21, low active */
	QSPI2_SSOC.B.OEN = 2;
	QSPI2_SSOC.B.AOL = 0;

	/* Q=1,A=0,B=1,C=1 */
	QSPI2_ECON1.B.A = 0;
	QSPI2_ECON1.B.B = 1;
	QSPI2_ECON1.B.C = 1;
	QSPI2_ECON1.B.Q = 1;
	QSPI2_ECON1.B.BE = 0;
	QSPI2_ECON1.B.CPH = 0;
	QSPI2_ECON1.B.CPOL = 0;
	QSPI2_ECON1.B.PAREN = 0;

	do{
		/* PT2F,PT1F,RXF,TXF,ERRORFLAGS */
		QSPI2_FLAGSCLEAR.B.ERRORCLEARS = 0x1F;
		QSPI2_FLAGSCLEAR.B.PT1C = 1;
		QSPI2_FLAGSCLEAR.B.PT2C = 1;
		QSPI2_FLAGSCLEAR.B.RXC = 1;
		QSPI2_FLAGSCLEAR.B.TXC = 1;
		QSPI2_FLAGSCLEAR.B.USRC = 1;
	} while (QSPI2_STATUS.U & 0xFFF);

	/* prepare data transfer format */
	/* CS=1,DL=15,MSB=1,TRAIL=1,LAST=1 */
	QSPI2_BACONENTRY.B.E = 0x17A10001;

	QSPI2_GLOBALCON.B.EN = 1;		/* ... and enable the module */

	/* command sequence for disabling external watchdog */
	//	const uint16_t wdtdiscmd[] =
	//	{
	//			0x8756, 0x87de, 0x86ad, 0x8625,		/* unprotect register (PROTCFG) */
	//			0x8d27,								/* disable window watchdog */
	//			0x8811,								/* disable err pin monitor (A-step) */
	//			0x8A01,								/* disable err pin monitor (not A-step) */
	//			0x87be, 0x8668, 0x877d, 0x8795		/* protect register (PROTCFG) */
	//	};
	tlf35584_cmd_t wdtdiscmd[]={
			// unprotect register (PROTCFG)
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0xAB,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0xEF,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0x56,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0x12,
					.parity = 0
			}},

			// disable window watchdog
			{.B={
					.cmd = 1,
					.addr = 6,
					.data = 0x93,
					.parity = 0
			}},

			//disable err pin monitor (A-step)
			{.B={
					.cmd = 1,
					.addr = 4,
					.data = 0x08,
					.parity = 0
			}},

			//disable err pin monitor (not A-step)
			{.B={
					.cmd = 1,
					.addr = 5,
					.data = 0x00,
					.parity = 0
			}},

			//protect register (PROTCFG)
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0xDF,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0x34,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0xBE,
					.parity = 0
			}},
			{.B={
					.cmd = 1,
					.addr = 3,
					.data = 0xCA,
					.parity = 0
			}},
	};

	/* transfer all data */
	for (uint8_t i = 0; i < sizeof(wdtdiscmd)/sizeof(wdtdiscmd[0]); ++i) {
		pack32 z_parity;
		z_parity.u32 = Ifx_PARITY((uint32_t)wdtdiscmd[i].U);
		wdtdiscmd[i].B.parity = z_parity.u8[0] ^ z_parity.u8[1];

		QSPI2_DATAENTRY0.B.E = (uint32_t)wdtdiscmd[i].U;
		/* wait until transfer is complete */
		while (!QSPI2_STATUS.B.TXF)
			;
		/* clear TX flag */
		QSPI2_FLAGSCLEAR.B.TXC = 1;
		/* wait for receive is finished */
		while (!QSPI2_STATUS.B.RXF)
			;
		/* clear RX flag */
		QSPI2_FLAGSCLEAR.B.RXC = 1;
		/* read and discard value */
		(void)QSPI2_RXEXIT.U;
	}
}

/* STM time scaling (for avoiding overflow) */
#define TIME_SCALE_DN		100
#define TIME_SCALE_UP		(1000000 / TIME_SCALE_DN)
/* wait for <time> micro seconds */

/* beware of overflows: 100 us at (>=)43 MHz will overflow (if not scaled before multiplying) */
void stm_wait(uint32_t us)
{
	uint32_t fSTM = (uint32_t)SYSTEM_GetStmClock();
	uint32_t stmWaitCount = (fSTM / TIME_SCALE_DN) * us / TIME_SCALE_UP;

	/* prepare compare register */
	STM0_CMP0.U = STM0_TIM0.U + stmWaitCount;
	STM0_CMCON.B.MSIZE0 = 31;
	/* Attention: keep this order, otherwise first match will trigger too soon */
	/* reset interrupt flag */
	STM0_ISCR.B.CMP0IRR = 1;
	/* enable compare match */
	STM0_ICR.B.CMP0EN = 1;
	/* wait for compare match */
	while (0 == STM0_ICR.B.CMP0IR)
	{
		;
	}
	STM0_ICR.B.CMP0EN = 0;
}

static void system_set_pll(const PllInitValue_t *pPllInitValue)
{
	unlock_safety_wdtcon();

	MODULE_SCU.OSCCON.U = pPllInitValue->valOSCCON;

	while (MODULE_SCU.CCUCON1.B.LCK)
		;
	MODULE_SCU.CCUCON1.U = pPllInitValue->valCCUCON1 | (1 << IFX_SCU_CCUCON1_UP_OFF);

	while (MODULE_SCU.CCUCON2.B.LCK)
		;
	MODULE_SCU.CCUCON2.U = pPllInitValue->valCCUCON2 | (1 << IFX_SCU_CCUCON2_UP_OFF);

	MODULE_SCU.PLLCON0.U |= ((1 << IFX_SCU_PLLCON0_VCOBYP_OFF) | (1 << IFX_SCU_PLLCON0_SETFINDIS_OFF));
	MODULE_SCU.PLLCON1.U =  pPllInitValue->valPLLCON1;				/* set Kn divider */
	MODULE_SCU.PLLCON0.U =  pPllInitValue->valPLLCON0				/* set P,N divider */
			| ((1 << IFX_SCU_PLLCON0_VCOBYP_OFF) | (1 << IFX_SCU_PLLCON0_CLRFINDIS_OFF));

	while (MODULE_SCU.CCUCON0.B.LCK)
		;
	MODULE_SCU.CCUCON0.U =  pPllInitValue->valCCUCON0 | (1 << IFX_SCU_CCUCON0_UP_OFF);

	lock_safety_wdtcon();

	if (0 == (pPllInitValue->valPLLCON0 & (1 << IFX_SCU_PLLCON0_VCOBYP_OFF)))	/* no prescaler mode requested */
	{
#ifndef SYSTEM_PLL_HAS_NO_LOCK
		/* wait for PLL locked */
		while (0 == MODULE_SCU.PLLSTAT.B.VCOLOCK)
			;
#endif

		unlock_safety_wdtcon();
		MODULE_SCU.PLLCON0.B.VCOBYP = 0;			/* disable VCO bypass */
		lock_safety_wdtcon();
	}

	/* update K dividers for stepping up to final clock */
	uint32_t k = MODULE_SCU.PLLCON1.B.K2DIV;
	/* wait some time (100 us) */
	stm_wait(100);
	while (k > pPllInitValue->finalK)
	{
		Ifx_SCU_PLLCON1 pllcon1 = MODULE_SCU.PLLCON1;

		--k;
		/* prepare value to write */
		pllcon1.B.K2DIV = k;
		pllcon1.B.K3DIV = k;
		/* wait until K2 operation is stable */
		while (0 == MODULE_SCU.PLLSTAT.B.K2RDY)
			;
		unlock_safety_wdtcon();
		MODULE_SCU.PLLCON1 = pllcon1;
		lock_safety_wdtcon();
		/* wait some time (100 us) */
		stm_wait(100);
	}
}

void system_clk_config_200_100(void) {
	//	200/100 MHz @ 20MHz ext. clock
	static const PllInitValue_t pll_init_200_100 = {
			.valOSCCON = 0x0007001C,
			.valPLLCON0 = 0x01017600,
			.valPLLCON1 = 0x00020505,
			.valCCUCON0 = 0x12120118,
			.valCCUCON1 = 0x10012242,
			.valCCUCON2 = 0x00000002,
			.finalK = 2
	};

	system_set_pll(&pll_init_200_100);
}

void system_clk_config_100_50(void) {
	// 100/50 MHz @ 20MHz ext. clock
	static const PllInitValue_t pll_init_100_50 = {
			.valOSCCON = 0x0007001C,
			.valPLLCON0 = 0x01018A00,
			.valPLLCON1 = 0x00020606,
			.valCCUCON0 = 0x12120118,
			.valCCUCON1 = 0x10012241,
			.valCCUCON2 = 0x00000002,
			.finalK = 6
	};

	system_set_pll(&pll_init_100_50);
}

void SYSTEM_Init(void) {
	disable_external_watchdog();
}

uint32_t system_GetPllClock(void) {
	uint32_t frequency = EXTCLK;	/* fOSC */

	Ifx_SCU_PLLSTAT pllstat = MODULE_SCU.PLLSTAT;
	Ifx_SCU_PLLCON0 pllcon0 = MODULE_SCU.PLLCON0;
	Ifx_SCU_PLLCON1 pllcon1 = MODULE_SCU.PLLCON1;

	if (0 == (pllstat.B.VCOBYST)) {
		if (0 == (pllstat.B.FINDIS)) {
			/* normal mode */
			frequency *= (pllcon0.B.NDIV + 1);		/* fOSC*N */
			frequency /= (pllcon0.B.PDIV + 1);		/* .../P  */
			frequency /= (pllcon1.B.K2DIV + 1);		/* .../K2 */
		} else {
			/* freerunning mode */
			frequency = 800000000;		/* fVCOBASE 800 MHz (???) */
			frequency /= (pllcon1.B.K2DIV + 1);		/* .../K2 */
		}
	} else {
		/* prescaler mode */
		frequency /= (pllcon1.B.K1DIV + 1);		/* fOSC/K1 */
	}

	return (uint32_t)frequency;
}

uint32_t system_GetIntClock(void) {
	uint32_t frequency = 0;
	switch (MODULE_SCU.CCUCON0.B.CLKSEL) {
	default:
	case 0:  /* back-up clock (typ. 100 MHz) */
		frequency = 100000000ul;
		break;
	case 1:	 /* fPLL */
		frequency = system_GetPllClock();
		break;
	}
	return frequency;
}

uint32_t SYSTEM_GetCpuClock(void) {
	uint32_t frequency = system_GetIntClock();
	/* fCPU = fSRI */
	uint32_t divider = MODULE_SCU.CCUCON0.B.SRIDIV;
	uint32_t cpudiv = MODULE_SCU.CCUCON6.B.CPU0DIV;
	if (0 == divider) {
		return 0;
	}

	frequency /= divider;

	if (cpudiv != 0) {
		frequency *= (64 - cpudiv);
		frequency /= 64;
	} else {
		return frequency;
	}
}

uint32_t SYSTEM_GetSysClock(void) {
	uint32_t frequency = system_GetIntClock();
	uint32_t divider = MODULE_SCU.CCUCON0.B.SPBDIV;
	if (0 == divider) {
		return 0;
	} else {
		return (frequency / divider);
	}
}

uint32_t SYSTEM_GetStmClock(void) {
	uint32_t frequency = system_GetIntClock();
	uint32_t divider = MODULE_SCU.CCUCON1.B.STMDIV;
	if (0 == divider) {
		return 0;
	} else {
		return (frequency / divider);
	}
}

uint32_t SYSTEM_GetCanClock(void) {
	uint32_t frequency = system_GetIntClock();
	uint32_t divider = MODULE_SCU.CCUCON1.B.CANDIV;
	if (0 == divider) {
		return 0;
	} else {
		return (frequency / divider);
	}
}

void SYSTEM_EnableProtectionExt(int Sel) {
	if (Sel < 3) {
		lock_wdtcon();			/* CPU watchdog */
	} else {
		lock_safety_wdtcon();	/* security watchdog */
	}
}

void SYSTEM_DisableProtectionExt(int Sel) {
	if (Sel < 3) {
		unlock_wdtcon();		/* CPU watchdog */
	} else {
		unlock_safety_wdtcon();	/* security watchdog */
	}
}

void SYSTEM_EnableSecProtection(void) {
	lock_safety_wdtcon();
}

void SYSTEM_DisableSecProtection(void) {
	unlock_safety_wdtcon();
}

void SYSTEM_Reset(void) {
	unlock_safety_wdtcon();
	MODULE_SCU.SWRSTCON.B.SWRSTREQ = 1;
	while (1)
		;
}

void SYSTEM_IdleExt(int CoreId) {
	unlock_wdtcon();
	MODULE_SCU.PMCSR[0].U = 1;	/* request CPU idle mode */
	lock_wdtcon();
}

void SYSTEM_Sleep(void) {
	unlock_wdtcon();
	MODULE_SCU.PMCSR[0].U = 2;	/* request system sleep mode */
	lock_wdtcon();
}

int SYSTEM_IsCacheEnabled(void) {
	uint32_t ui = _mfcr(CPU_PCON0);
	if (ui & 2) {
		return 0;	/* Cache is in bypass mode */
	}

	ui = _mfcr(CPU_PCON2);
	if (0 == (ui & (IFX_CPU_PCON2_PCACHE_SZE_MSK << IFX_CPU_PCON2_PCACHE_SZE_OFF))) {
		return 0;	/* Cache size is 0 */
	} else {
		return 1;
	}
}

void SYSTEM_EnaDisCache(int Enable) {
	unlock_wdtcon();
	if (Enable) {
		_mtcr(CPU_PCON0, 0);
		_mtcr(CPU_DCON0, 0);
	} else {
		_mtcr(CPU_PCON0, 2);
		_mtcr(CPU_PCON1, 3);
		_mtcr(CPU_DCON0, 2);
	}
	lock_wdtcon();
}
