/*====================================================================
* Project:  Board Support Package (BSP) examples
* Function: example using a serial line (interrupt mode)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>

#include "timer.h"

#ifndef MSC_CLOCK
#define MSC_CLOCK

#define HZ	1000
#endif

#include "led.h"
#include "uart_int.h"

#ifdef __TRICORE__
#if !defined(__TC161__) && !defined(__TC162__)
#include <machine/intrinsics.h>
#endif /* !__TC161__ && !__TC162__ */

#if defined(__TC161__)
#include "system_tc2x.h"
#endif /* __TC161__  */

#if defined(__TC162__)
#include "system_tc3x.h"
#endif /* __TC162__  */
#endif /* __TRICORE__ */

#define SPRINTF		sprintf
#define VSPRINTF	vsprintf

#define BUFSIZE		0x100

#define BAUDRATE	115200

volatile uint32_t g_Ticks;

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_Ticks;
}

static void my_puts(const char *str)
{
	char buffer[BUFSIZE];

	SPRINTF(buffer, "%s\r\n", str);
	_uart_puts(buffer);
}

/* POSIX read function */
/* read characters from file descriptor fd into given buffer, at most count bytes */
/* returns number of characters in buffer */
size_t read(int fd, void *buffer, size_t count)
{
	size_t index = 0;

	if (fileno(stdin) == fd)
	{
#if (NON_BLOCKING_SERIALIO > 0)
		char *ptr = (char *)buffer;
		do
		{
			if (1 == _uart_getchar(ptr))
			{
				++ptr;
				++index;
			}
			else
			{
				/* wait at least for 1 character */
				if (index >= 1)
				{
					break;
				}
			}
		} while (index < count);
#else
		unsigned char *ptr = (unsigned char *)buffer;
		do
		{
			if (1 == _poll_uart(ptr))
			{
				++ptr;
				++index;
			}
			else
			{
				/* wait at least for 1 character */
				if (index >= 1)
				{
					break;
				}
			}
		} while (index < count);
#endif /* NON_BLOCKING_SERIALIO */
	}

	return index;
}

/* POSIX write function */
/* write content of buffer to file descriptor fd */
/* returns number of characters that have been written */
size_t write(int fd, const void *buffer, size_t count)
{
	size_t index = 0;

	if ((fileno(stdout) == fd) || (fileno(stderr) == fd))
	{
#if (NON_BLOCKING_SERIALIO > 0)
		int ret = _uart_send((const char *)buffer, (int)count);
		if (ret)
		{
			index = count;
		}
#else
		const unsigned char *ptr = (const unsigned char *)buffer;
		while (index < count)
		{
			_uart_puts(*ptr++);
			++index;
		}
#endif /* NON_BLOCKING_SERIALIO */
	}

	return index;
}

int main(void)
{
	char c;
	int quit = 0;

#ifdef __TRICORE__
#if defined(__TC161__) || defined(__TC162__)
	SYSTEM_Init();
#endif /* __TC161__ || __TC162__ */
#endif /* __TRICORE__ */

	_init_uart(BAUDRATE);
	InitLED();

#ifdef __TRICORE__
#if !defined(__TC161__) && !defined(__TC162__)
	/* enable global interrupts */
	_enable();
#endif /* !__TC161__ && !__TC162__ */
#endif /* __TRICORE__ */


	/* initialise timer at SYSTIME_CLOCK rate */
	TimerInit(HZ);
	/* add own handler for timer interrupts */
	TimerSetHandler(my_timer_handler);

	/* enable global interrupts */
	_enable();

	printf("Complex Test @ Sys:%u Hz CPU:%u Hz CoreType:%04X\n",
			SYSTEM_GetSysClock(),
			SYSTEM_GetCpuClock(),
			__TRICORE_CORE__
	);
	while (!quit)
	{
		if (_uart_getchar(&c))
		{
			switch (c)
			{
				case '0' :
					LEDOFF(0);
					my_puts("LED switched to OFF");
					break;
				case '1' :
					LEDON(0);
					my_puts("LED switched to ON");
					break;
				case '2' :
					printf("%s,%s\n", __DATE__, __TIME__);
					break;
//				case 'E' :
//					quit = 1;
//					my_puts("Bye bye!");
//					break;
				case '\n' :
				case '\r' :
					/* do nothing -- ignore it */
					break;
				default :
					printf("Command '%c' not supported\r\n", c);
					break;
			}
		}
	}

	/* wait until sending has finished */
	while (_uart_sending())
		;

	return EXIT_SUCCESS;
}
