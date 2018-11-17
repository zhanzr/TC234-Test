/*====================================================================
* Project:  Board Support Package (BSP)
* Function: LEDs
*
*====================================================================*/

#ifndef __LED_H__
#define __LED_H__

#include "bspconfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* number of available LEDs */
#define MAX_LED					4

void led_on(uint8_t n);
void led_off(uint8_t n);
void led_toggle(uint8_t n);
void led_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LED_H__ */
