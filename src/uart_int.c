/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line.
*           (interrupt variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2015
*====================================================================*/
#define MODULE_UART_INT

#include <string.h>

#include "bspconfig.h"
#include "uart_int.h"


#ifndef RS232_RX_BUFSIZE
#define RS232_RX_BUFSIZE	0x100
#endif /* RS232_RX_BUFSIZE */

#ifndef RS232_TX_BUFSIZE
#define RS232_TX_BUFSIZE	0x200
#endif /* RS232_TX_BUFSIZE */

#ifndef RX_CLEAR
#define RX_CLEAR(u)
#endif

#ifndef TX_CLEAR
#define TX_CLEAR(u)
#endif


typedef struct
{
	unsigned int	head;
	unsigned int	tail;
	char			buf[RS232_RX_BUFSIZE];
} RxBuffer_t;


typedef struct
{
	unsigned int	head;
	unsigned int	tail;
	char			buf[RS232_TX_BUFSIZE];
} TxBuffer_t;


/* Circular send and receive buffers */
static TxBuffer_t sendBuf;
static RxBuffer_t recvBuf;


/* FIFO support */
static __inline int isEmptyTXFifo(void)
{
	return (sendBuf.tail == sendBuf.head);
}

static __inline int getFreeTXFifo(void)
{
	int used = (RS232_TX_BUFSIZE + sendBuf.head - sendBuf.tail) % RS232_TX_BUFSIZE;
	return (RS232_TX_BUFSIZE - 1 - used);
}

static __inline int readTXFifo(char *cp)
{
	int res = 0;
	if (sendBuf.tail != sendBuf.head)
	{
		unsigned int next = (sendBuf.tail + 1) % RS232_TX_BUFSIZE;
		*cp = sendBuf.buf[sendBuf.tail];
		sendBuf.tail = next;
		res = 1;
	}
	return res;
}

static __inline int writeTXFifo(char c)
{
	int res = 0;
	unsigned int next = (sendBuf.head + 1) % RS232_TX_BUFSIZE;
	if (next != sendBuf.tail)
	{
		sendBuf.buf[sendBuf.head] = c;
		sendBuf.head = next;
		res = 1;
	}
	return res;
}

static __inline int readRXFifo(char *cp)
{
	int res = 0;
	if (recvBuf.tail != recvBuf.head)
	{
		unsigned int next = (recvBuf.tail + 1) % RS232_RX_BUFSIZE;
		*cp = recvBuf.buf[recvBuf.tail];
		recvBuf.tail = next;
		res = 1;
	}
	return res;
}

static __inline int writeRXFifo(char c)
{
	int res = 0;
	unsigned int next = (recvBuf.head + 1) % RS232_RX_BUFSIZE;
	if (next != recvBuf.tail)
	{
		recvBuf.buf[recvBuf.head] = c;
		recvBuf.head = next;
		res = 1;
	}
	return res;
}


/* Send character CHR via the serial line */
static __inline void _out_uart(const char chr)
{
	TX_CLEAR(UARTBASE);
	/* send the character */
	PUT_CHAR(UARTBASE, chr);
}

/* Receive (and return) a character from the serial line */
static __inline char _in_uart(void)
{
	/* read the character */
	char c = (char)GET_CHAR(UARTBASE);
	/* acknowledge receive */
	RX_CLEAR(UARTBASE);
	return c;
}

/* Interrupt Service Routine for RX */
static void _uart_rx_handler(int arg)
{
	(void)arg;
	/* check for error condition */
	if (GET_ERROR_STATUS(UARTBASE))
	{
		/* ignore this character */
		_in_uart();
		/* reset error flags */
		RESET_ERROR(UARTBASE);
	}
	else
	{
		char c = _in_uart();
		writeRXFifo(c);
	}
}

/* Interrupt Service Routine for TX */
static void _uart_tx_handler(int arg)
{
	char c;
	(void)arg;

	if (readTXFifo(&c))
	{
		_out_uart(c);
	}
	else
	{
		/* all done --> disable TX interrupt */
		TX_INT_STOP(UARTBASE);
	}
}


/* Externally visible functions */


/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
void _init_uart(int baudrate)
{
    _uart_init_bsp(baudrate, _uart_rx_handler, _uart_tx_handler);
}

/* get a character from serial line */
int _uart_getchar(char *c)
{
	return readRXFifo(c);
}

/* send a buffer of given size <len> over serial line */
int _uart_send(const char *buffer, int len)
{
	int ret = 0;

	if (len)
	{
		if (getFreeTXFifo() >= len)
		{
			int cnt = 0;
			for (; cnt < len; ++cnt)
			{
				writeTXFifo(*buffer++);
			}
			/* check whether TX must be triggered */
			if (!TX_INT_CHECK(UARTBASE))
			{
				/* enable TX interrupt for sending */
				TX_INT_START(UARTBASE);
			}
			ret = 1;
		}
	}

	return ret;
}

/* send a string over serial line */
int _uart_puts(const char *str)
{
	int len = strlen(str);

	return _uart_send(str, len);
}

/* test UARTs sending state */
int _uart_sending(void)
{
	int ret = (0 == isEmptyTXFifo());
	if (0 == ret)
	{
		/* wait until last byte is sent */
		ret = TX_INT_CHECK(UARTBASE);
	}
	return ret;
}

size_t read(int fd, void *buffer, size_t count)
{
	//Dummy read, don't use scanf
	return count;
}

size_t write(int fd, const void *buffer, size_t count)
{
	_uart_send(buffer, count);

	return count;
}
