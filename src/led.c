#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "led.h"
#include "uart_int.h"

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#include "system_tc2x.h"
#include "machine/intrinsics.h"
#include "interrupts.h"

/* AppKit-TC2X4: P13.0 .. P13.3 --> LED D107 ... D110 */
static Ifx_P * const portLED = (Ifx_P *)&MODULE_P13;

/* OMR is WO ==> don't use load-modify-store access! */
/* set PSx pin */
#define LED_PIN_SET(x)			(1 << (LED_PIN_NR + (x)))
/* set PCLx pin */
#define LED_PIN_RESET(x)		(1 << (LED_PIN_NR + IFX_P_OMR_PCL0_OFF + (x)))

void LEDON(int nr)
{
	if (nr < MAX_LED)
	{
		portLED->OMR.U = LED_PIN_RESET(nr);
	}
}

void LEDOFF(int nr)
{
	if (nr < MAX_LED)
	{
		portLED->OMR.U = LED_PIN_SET(nr);
	}
}

void LEDTOGGLE(int nr)
{
	if (nr < MAX_LED)
	{
		/* set PCLx and PSx pin to 1 ==> toggle pin state */
		portLED->OMR.U = LED_PIN_RESET(nr) | LED_PIN_SET(nr);
	}
}

void InitLED(void)
{
	/* initialise all LEDs (P13.0 .. P13.3) */
	portLED->IOCR0.U = 0x80808080;	/* OUT_PPGPIO */
	/* all LEDs OFF */
	portLED->OMR.U = (MASK_ALL_LEDS << LED_PIN_NR);
}
