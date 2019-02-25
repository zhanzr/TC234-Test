/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line
*           (interrupt variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2015
*====================================================================*/

#ifndef __UART_INT_H__
#define __UART_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define BAUDRATE	115200

void uart_init(uint32_t baudrate);
int _uart_send(const char *buffer, int len);
int _uart_puts(const char *str);
int _uart_getchar(char *c);
int _uart_sending(void);

static inline void simple_delay(uint32_t t) {
	for(uint32_t i=0; i<t; ++i) {
		__asm__ volatile ("nop" ::: "memory");
		__asm volatile ("" : : : "memory");
	}
}

static inline void flush_stdout_trap(void) {
	simple_delay(10000);
}

static inline void flush_stdout(void) {
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending()) {
		;
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_INT_H__ */
