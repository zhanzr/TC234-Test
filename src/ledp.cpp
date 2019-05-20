/*
 * ledp.cpp
 *
 *  Created on: May 20, 2019
 *      Author: Administrator
 */

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

#include "ledp.hpp"
#include "uart_int.h"
#include "system_tc2x.h"
#include "interrupts_tc23x.h"

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)

static void led_on(uint8_t n) {
	switch (n) {
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

static void led_off(uint8_t n) {
	switch (n) {
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

//Note LEDs are negative logic controlled
static uint32_t led_stat(uint8_t n) {
	switch (n) {
	case 0:
		return PORT_LED.IN.B.P0;
		break;
	case 1:
		return PORT_LED.IN.B.P1;
		break;
	case 2:
		return PORT_LED.IN.B.P2;
		break;
	case 3:
		return PORT_LED.IN.B.P3;
		break;

	default:
		break;
	}
}

static void led_toggle(uint8_t n) {
	switch (n) {
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

//Default Constructor
LED::LED(void) {
	m_num = 0;
	m_has_init = true;
}

LED::LED(uint8_t n) {
	m_num = n;
	m_has_init = true;
}

LED::~LED(void) {
	//Do nothing currently
}

void LED::on(void) {
	::led_on(m_num);
}

void LED::off(void) {
	::led_off(m_num);
}

uint32_t LED::stat(void) {
	return ::led_stat(m_num);
}

void LED::toggle(void) {
	::led_toggle(m_num);
}

static void LED::init(void) {
	// Initialize all LEDs (P13.0 .. P13.3) -> D107 ... D110
	// The LEDs are controlled by negative logic, so both of open-drain output and push-pull work
	// But only in push-pull output method, the pin state could be read back via IN register
	PORT_LED.IOCR0.B.PC0 = OUT_PPGPIO;
	PORT_LED.IOCR0.B.PC1 = OUT_PPGPIO;
	PORT_LED.IOCR0.B.PC2 = OUT_PPGPIO;
	PORT_LED.IOCR0.B.PC3 = OUT_PPGPIO;

	/* all LEDs OFF */
	PORT_LED.OMR.B.PS0 = 1;
	PORT_LED.OMR.B.PS1 = 1;
	PORT_LED.OMR.B.PS2 = 1;
	PORT_LED.OMR.B.PS3 = 1;
}


static inline void simple_delay(uint32_t d)
{
	for(uint32_t i=0; i<d; ++i) {
		__asm__ volatile ("nop" ::: "memory");
		__asm volatile ("" : : : "memory");
	}
}

void test_led_cpp(void) {
	printf("%s\n", __func__);
	LED* led_p_s[LED::MAX_LED];
	for(uint8_t i=0; i<LED::MAX_LED; ++i) {
		led_p_s[i] = new LED(i);
	}

	printf("after object instanization\n");

	for(uint8_t i=0; i<LED::MAX_LED; ++i) {
		led_p_s[i]->on();
		simple_delay(10000);
	}
	simple_delay(10000);

	for(uint8_t i=0; i<LED::MAX_LED; ++i) {
		led_p_s[i]->off();
		simple_delay(10000);
	}
	simple_delay(10000);

	led_p_s[0]->on();
	led_p_s[2]->on();
	for(uint8_t i=0; i<LED::MAX_LED; ++i) {
		led_p_s[i]->toggle();
		simple_delay(10000);
	}
	simple_delay(10000);

	for(uint8_t i=0; i<LED::MAX_LED; ++i) {
		led_p_s[i]->off();
		simple_delay(10000);
	}
	simple_delay(10000);

	printf("after test\n");
}
