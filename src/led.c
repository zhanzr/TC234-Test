#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "machine/intrinsics.h"

#include "led.h"
#include "uart_int.h"
#include "system_tc2x.h"
#include "interrupts_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)

/* AppKit-TC2X4: P13.0 .. P13.3 --> LED D107 ... D110 */
#define	PORT_LED	MODULE_P13

void led_on(uint8_t n)
{
	switch (n)
	{
	case 0:
		PORT_LED.OMR.B.PCL0 = 1;
		break;
	case 1:
		PORT_LED.OMR.B.PCL1 = 1;
		break;
	case 2:
		PORT_LED.OMR.B.PCL2 = 1;
		break;
	case 3:
		PORT_LED.OMR.B.PCL3 = 1;
		break;

	default:
		break;
	}
}

void led_off(uint8_t n)
{
	switch (n)
	{
	case 0:
		PORT_LED.OMR.B.PS0 = 1;
		break;
	case 1:
		PORT_LED.OMR.B.PS1 = 1;
		break;
	case 2:
		PORT_LED.OMR.B.PS2 = 1;
		break;
	case 3:
		PORT_LED.OMR.B.PS3 = 1;
		break;

	default:
		break;
	}
}

void led_toggle(uint8_t n)
{
	switch (n)
	{
	case 0:
		PORT_LED.OMR.U |= ((1<<IFX_P_OMR_PS0_OFF)|(1<<IFX_P_OMR_PCL0_OFF));
		break;
	case 1:
		PORT_LED.OMR.U |= ((1<<IFX_P_OMR_PS1_OFF)|(1<<IFX_P_OMR_PCL1_OFF));
		break;
	case 2:
		PORT_LED.OMR.U |= ((1<<IFX_P_OMR_PS2_OFF)|(1<<IFX_P_OMR_PCL2_OFF));
		break;
	case 3:
		PORT_LED.OMR.U |= ((1<<IFX_P_OMR_PS3_OFF)|(1<<IFX_P_OMR_PCL3_OFF));
		break;

	default:
		break;
	}
}

void led_init(void)
{
	/* initialise all LEDs (P13.0 .. P13.3) */
	PORT_LED.IOCR0.B.PC0 = OUT_ODGPIO;
	PORT_LED.IOCR0.B.PC1 = OUT_ODGPIO;
	PORT_LED.IOCR0.B.PC2 = OUT_ODGPIO;
	PORT_LED.IOCR0.B.PC3 = OUT_ODGPIO;

	/* all LEDs OFF */
	PORT_LED.OMR.B.PS0 = 1;
	PORT_LED.OMR.B.PS1 = 1;
	PORT_LED.OMR.B.PS2 = 1;
	PORT_LED.OMR.B.PS3 = 1;
}
