/*====================================================================
* Project:  Board Support Package (BSP)
* Function: LEDs
*
* Copyright HighTec EDV-Systeme GmbH 1982-2015
*====================================================================*/

#ifndef __LED_H__
#define __LED_H__

#include "bspconfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


static __inline void LEDON(int nr)
{
	if (nr < MAX_LED)
	{
		LED_ON(nr);
	}
}

static __inline void LEDOFF(int nr)
{
	if (nr < MAX_LED)
	{
		LED_OFF(nr);
	}
}

static __inline void LEDTOGGLE(int nr)
{
	if (nr < MAX_LED)
	{
		LED_TOGGLE(nr);
	}
}


static __inline void InitLED(void)
{
	INIT_LEDS;
}

/* type of a timer callback function */
typedef void (*TCF)(void);

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz);

/* Install <handler> as timer callback function */
void TimerSetHandler(TCF handler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LED_H__ */
