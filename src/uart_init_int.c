/*====================================================================
 * Project:  Board Support Package (BSP)
 * Function: Transmit and receive characters via TriCore's serial line
 *           (Infineon controller TC23xx)
 *           (interrupt variant)
 *
 * Copyright HighTec EDV-Systeme GmbH 1982-2016
 *====================================================================*/
#define MODULE_UART_INT

#include "bspconfig_tc23x.h"

#include <machine/wdtcon.h>

/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
void _uart_init_bsp(int baudrate, void (*uart_rx_isr)(int arg), void (*uart_tx_isr)(int arg)) {
	/* install handlers for transmit and receive interrupts */
	InterruptInstall(SRC_ID_ASCLIN0TX, uart_tx_isr, TXC_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_ASCLIN0RX, uart_rx_isr, RXD_ISR_PRIO, 0);

	/* on board wiggler is connected to ASCLIN0 */
	/* ARX0A/P14.1 (RXD), ATX0/P14.0 (TXD) */
	/* Set TXD/P14.0 to "output" and "high" */
	port_UART->IOCR0.B.PC0 = OUT_PPALT2;
	port_UART->OMR.B.PS0 = 1;

	uint32_t numerator;
	switch (baudrate) {
	case   9600 :
		numerator = NUMERATOR_BAUD_9600;
		break;

	case  19200 :
		numerator = NUMERATOR_BAUD_19200;
		break;

	case  38400 :
		numerator = NUMERATOR_BAUD_38400;
		break;

	case  57600 :
		numerator = NUMERATOR_BAUD_57600;
		break;

	case 115200 :
		numerator =NUMERATOR_BAUD_115200;
		break;

	default:
		break;
	}

	/* Enable ASCn */
	unlock_wdtcon();
	UARTBASE->CLC.U = 0;
	lock_wdtcon();
	/* read back for activating module */
	(void)UARTBASE->CLC.U;

	/* select ARX0A/P14.1 as input pin */
	UARTBASE->IOCR.B.ALTI = IN_NOPULL0;

	/* Program ASC0 */
	UARTBASE->CSR.U = 0;

	/* configure TX and RX FIFOs */
	UARTBASE->TXFIFOCON.U = (1 << 6)	/* INW: (1 == 1 byte) */
								  | (1 << 1)	/* ENO */
								  | (1 << 0);	/* FLUSH */
	UARTBASE->RXFIFOCON.U = (1 << 6)	/* OUTW: (1 == 1 byte) */
								  | (1 << 1)	/* ENI */
								  | (1 << 0);	/* FLUSH */

	UARTBASE->BITCON.U = ( 9 << 0)		/* PRESCALER: 10 */
							   | (15 << 16)		/* OVERSAMPLING: 16 */
							   | ( 9 << 24)		/* SAMPLEPOINT: position 7,8,9 */
							   | (1u << 31);	/* SM: 3 samples per bit */

	/* data format: 8N1 */
	UARTBASE->FRAMECON.U = (1 << 9)		/* STOP: 1 bit */
								 | (0 << 16)	/* MODE: Init */
								 | (0 << 30);	/* PEN: no parity */
	UARTBASE->DATCON.B.DATLEN = 7;		/* DATLEN: 8 bit */

	uint32_t clk_sys = SYSTEM_GetSysClock();
	/* baudrate values at 100 MHz:3125 */
	uint32_t denominator = clk_sys/32000;
	/* set baudrate value */
	UARTBASE->BRG.B.DENOMINATOR = denominator;
	UARTBASE->BRG.B.NUMERATOR = numerator;

	UARTBASE->FRAMECON.B.MODE = 1;			/* ASC mode */
	UARTBASE->FLAGSENABLE.B.RFLE = 1;
	UARTBASE->CSR.B.CLKSEL = 1;					/* select CLC as clock source */
}
