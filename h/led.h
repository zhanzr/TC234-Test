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

#define MAX_LED					4	/* number of available LEDs */
#define LED_PIN_NR			 	0	/* pin number of first used LED */

#define MASK_ALL_LEDS			((1 << MAX_LED) - 1)

 void LEDON(int nr);
 void LEDOFF(int nr);
 void LEDTOGGLE(int nr);
 void InitLED(void);

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
