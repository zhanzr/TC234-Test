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

#define BAUDRATE	115200

void _init_uart(int baudrate);
int _uart_send(const char *buffer, int len);
int _uart_puts(const char *str);
int _uart_getchar(char *c);
int _uart_sending(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_INT_H__ */
