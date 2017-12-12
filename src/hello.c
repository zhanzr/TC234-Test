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
#include <math.h>

#include "led.h"
#include "uart_int.h"

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#ifdef __TRICORE__
#if !defined(__TC161__) && !defined(__TC162__)
#include <machine/intrinsics.h>
#endif /* !__TC161__ && !__TC162__ */

#if defined(__TC161__)
#include "system_tc2x.h"
#endif /* __TC161__  */

#endif /* __TRICORE__ */

#ifdef MINIMAL_CODE
#include "usr_sprintf.h"
#define SPRINTF		usr_sprintf
#define VSPRINTF	usr_vsprintf
#else
#define SPRINTF		sprintf
#define VSPRINTF	vsprintf
#endif /* MINIMAL_CODE */

#include "interrupts.h"

#define BUFSIZE		128

#define BAUDRATE	115200

#define SYSTIME_CLOCK	1000	/* timer event rate [Hz] */

static const char *my_str = "Hello world!";

volatile uint32_t g_SysTicks;
volatile uint8_t test_flag = 0;

/* Private variables ---------------------------------------------------------*/
/*
 * C Converted Whetstone Double Precision Benchmark
 *		Version 1.2	22 March 1998
 *
 *	(c) Copyright 1998 Painter Engineering, Inc.
 *		All Rights Reserved.
 *
 *		Permission is granted to use, duplicate, and
 *		publish this text and program as long as it
 *		includes this entire comment block and limited
 *		rights reference.
 *
 * Converted by Rich Painter, Painter Engineering, Inc. based on the
 * www.netlib.org benchmark/whetstoned version obtained 16 March 1998.
 *
 * A novel approach was used here to keep the look and feel of the
 * FORTRAN version.  Altering the FORTRAN-based array indices,
 * starting at element 1, to start at element 0 for C, would require
 * numerous changes, including decrementing the variable indices by 1.
 * Instead, the array E1[] was declared 1 element larger in C.  This
 * allows the FORTRAN index range to function without any literal or
 * variable indices changes.  The array element E1[0] is simply never
 * used and does not alter the benchmark results.
 *
 * The major FORTRAN comment blocks were retained to minimize
 * differences between versions.  Modules N5 and N12, like in the
 * FORTRAN version, have been eliminated here.
 *
 * An optional command-line argument has been provided [-c] to
 * offer continuous repetition of the entire benchmark.
 * An optional argument for setting an alternate LOOP count is also
 * provided.  Define PRINTOUT to cause the POUT() function to print
 * outputs at various stages.  Final timing measurements should be
 * made with the PRINTOUT undefined.
 *
 * Questions and comments may be directed to the author at
 *			r.painter@ieee.org
 */
/*
C**********************************************************************
C     Benchmark #2 -- Double  Precision Whetstone (A001)
C
C     o	This is a REAL*8 version of
C	the Whetstone benchmark program.
C
C     o	DO-loop semantics are ANSI-66 compatible.
C
C     o	Final measurements are to be made with all
C	WRITE statements and FORMAT sttements removed.
C
C**********************************************************************
*/
/* map the FORTRAN math functions, etc. to the C versions */
#define DSIN	sin
#define DCOS	cos
#define DATAN	atan
#define DLOG	log
#define DEXP	exp
#define DSQRT	sqrt
#define IF		if

/* function prototypes */
void POUT(long N, long J, long K, double X1, double X2, double X3, double X4);
void PA(double E[]);
void P0(void);
void P3(double X, double Y, double *Z);
#define USAGE	"usage: whetdc [-c] [loops]\n"

/*
	COMMON T,T1,T2,E1(4),J,K,L
*/
double T,T1,T2,E1[5];
int J,K,L;


void
PA(double E[])
{
	J = 0;

L10:
	E[1] = ( E[1] + E[2] + E[3] - E[4]) * T;
	E[2] = ( E[1] + E[2] - E[3] + E[4]) * T;
	E[3] = ( E[1] - E[2] + E[3] + E[4]) * T;
	E[4] = (-E[1] + E[2] + E[3] + E[4]) / T2;
	J += 1;

	if (J < 6)
		goto L10;
}

void
P0(void)
{
	E1[J] = E1[K];
	E1[K] = E1[L];
	E1[L] = E1[J];
}

void
P3(double X, double Y, double *Z)
{
	double X1, Y1;

	X1 = X;
	Y1 = Y;
	X1 = T * (X1 + Y1);
	Y1 = T * (X1 + Y1);
	*Z  = (X1 + Y1) / T2;
}

#ifdef PRINTOUT
void
POUT(long N, long J, long K, double X1, double X2, double X3, double X4)
{
	printf("%7ld %7ld %7ld %12.4e %12.4e %12.4e %12.4e\n",
						N, J, K, X1, X2, X3, X4);
}
#endif

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_SysTicks;
	if(0==g_SysTicks%SYSTIME_CLOCK)
	{
		test_flag = 1;
	}
}

const uint32_t HAL_GetTick(void)
{
	return g_SysTicks;
}

static void my_puts(const char *str)
{
	char buffer[BUFSIZE];

	SPRINTF(buffer, "%s\r\n", str);
	_uart_puts(buffer);
}

static void my_printf(const char *fmt, ...)
{
	char buffer[BUFSIZE];
	va_list ap;

	va_start(ap, fmt);
	VSPRINTF(buffer, fmt, ap);
	va_end(ap);

	_uart_puts(buffer);
}


#define SYSTIME_ISR_PRIO	2

#define STM0_BASE			((Ifx_STM *)&MODULE_STM0)

/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value = 0;

/* pointer to user specified timer callback function */
static TCF user_handler = (TCF)0;

static __inline Ifx_STM *systime_GetStmBase(void)
{
	switch (_mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK)
	{
		case 0 :
		default :
			return STM0_BASE;
		break;
	}
}

/* timer interrupt routine */
static void tick_irq(int reload_value)
{
	Ifx_STM *StmBase = systime_GetStmBase();

	/* set new compare value */
	StmBase->CMP[0].U += (unsigned int)reload_value;
	if (user_handler)
	{
		user_handler();
	}
}

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz)
{
	unsigned int CoreID = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	Ifx_STM *StmBase = systime_GetStmBase();
	unsigned int frequency = SYSTEM_GetStmClock();
	int irqId;

	reload_value = frequency / hz;

	switch (CoreID)
	{
		default :
		case 0 :
			irqId = SRC_ID_STM0SR0;
			break;
	}

	/* install handler for timer interrupt */
	InterruptInstall(irqId, tick_irq, SYSTIME_ISR_PRIO, (int)reload_value);

	/* reset interrupt flag */
	StmBase->ISCR.U = (IFX_STM_ISCR_CMP0IRR_MSK << IFX_STM_ISCR_CMP0IRR_OFF);
	/* prepare compare register */
	StmBase->CMP[0].U = StmBase->TIM0.U + reload_value;
	StmBase->CMCON.U  = 31;
	StmBase->ICR.B.CMP0EN = 1;
}

/* Install <handler> as timer callback function */
void TimerSetHandler(TCF handler)
{
	user_handler = handler;
}

int main(void)
{
	char c;
	int quit = 0;
	/* used in the FORTRAN version */
		long I;
		long N1, N2, N3, N4, N6, N7, N8, N9, N10, N11;
		double X1,X2,X3,X4,X,Y,Z;
		long LOOP;
		int II, JJ;

		/* added for this version */
		long loopstart;
	//	long startsec, finisec;
		uint32_t start_tick, fini_tick;
		float KIPS;
		int continuous;

	//On my host PC
	//Loops: 500000, Iterations: 1, Duration: 16 sec.
	//C Converted Double Precision Whetstones: 3125.0 MIPS
		loopstart = 2000;		/* see the note about LOOP below */
		continuous = 1;
		II = 1;		/* start at the first arg (temp use of II here) */

#ifdef __TRICORE__
#if defined(__TC161__) || defined(__TC162__)
	SYSTEM_Init();
#endif /* __TC161__ || __TC162__ */
#endif /* __TRICORE__ */

	/* initialise timer at SYSTIME_CLOCK rate */
	TimerInit(SYSTIME_CLOCK);
	/* add own handler for timer interrupts */
	TimerSetHandler(my_timer_handler);

	_init_uart(BAUDRATE);
	InitLED();

#ifdef __TRICORE__
#if !defined(__TC161__) && !defined(__TC162__)
	/* enable global interrupts */
	_enable();
#endif /* !__TC161__ && !__TC162__ */
#endif /* __TRICORE__ */

	printf("Whetstone for TC234 %d %d\n", sizeof(float), sizeof(double));
	printf("Sys:%u Hz, Cpu:%u Hz, Stm:%u Hz\n",
			SYSTEM_GetSysClock(),
			SYSTEM_GetCpuClock(),
			SYSTEM_GetStmClock()
			);


	LCONT:
	/*
	C
	C	Start benchmark timing at this point.
	C
	*/
		start_tick = HAL_GetTick();

	/*
	C
	C	The actual benchmark starts here.
	C
	*/
		T  = .499975;
		T1 = 0.50025;
		T2 = 2.0;
	/*
	C
	C	With loopcount LOOP=10, one million Whetstone instructions
	C	will be executed in EACH MAJOR LOOP..A MAJOR LOOP IS EXECUTED
	C	'II' TIMES TO INCREASE WALL-CLOCK TIMING ACCURACY.
	C
		LOOP = 1000;
	*/
		LOOP = loopstart;
		II   = 1;

		JJ = 1;

	IILOOP:
		N1  = 0;
		N2  = 12 * LOOP;
		N3  = 14 * LOOP;
		N4  = 345 * LOOP;
		N6  = 210 * LOOP;
		N7  = 32 * LOOP;
		N8  = 899 * LOOP;
		N9  = 616 * LOOP;
		N10 = 0;
		N11 = 93 * LOOP;
	/*
	C
	C	Module 1: Simple identifiers
	C
	*/
		X1  =  1.0;
		X2  = -1.0;
		X3  = -1.0;
		X4  = -1.0;

		for (I = 1; I <= N1; I++) {
		    X1 = (X1 + X2 + X3 - X4) * T;
		    X2 = (X1 + X2 - X3 + X4) * T;
		    X3 = (X1 - X2 + X3 + X4) * T;
		    X4 = (-X1+ X2 + X3 + X4) * T;
		}
	#ifdef PRINTOUT
		IF (JJ==II)POUT(N1,N1,N1,X1,X2,X3,X4);
	#endif

	/*
	C
	C	Module 2: Array elements
	C
	*/
		E1[1] =  1.0;
		E1[2] = -1.0;
		E1[3] = -1.0;
		E1[4] = -1.0;

		for (I = 1; I <= N2; I++) {
		    E1[1] = ( E1[1] + E1[2] + E1[3] - E1[4]) * T;
		    E1[2] = ( E1[1] + E1[2] - E1[3] + E1[4]) * T;
		    E1[3] = ( E1[1] - E1[2] + E1[3] + E1[4]) * T;
		    E1[4] = (-E1[1] + E1[2] + E1[3] + E1[4]) * T;
		}

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N2,N3,N2,E1[1],E1[2],E1[3],E1[4]);
	#endif

	/*
	C
	C	Module 3: Array as parameter
	C
	*/
		for (I = 1; I <= N3; I++)
			PA(E1);

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N3,N2,N2,E1[1],E1[2],E1[3],E1[4]);
	#endif

	/*
	C
	C	Module 4: Conditional jumps
	C
	*/
		J = 1;
		for (I = 1; I <= N4; I++) {
			if (J == 1)
				J = 2;
			else
				J = 3;

			if (J > 2)
				J = 0;
			else
				J = 1;

			if (J < 1)
				J = 1;
			else
				J = 0;
		}

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N4,J,J,X1,X2,X3,X4);
	#endif

	/*
	C
	C	Module 5: Omitted
	C 	Module 6: Integer arithmetic
	C
	*/

		J = 1;
		K = 2;
		L = 3;

		for (I = 1; I <= N6; I++) {
		    J = J * (K-J) * (L-K);
		    K = L * K - (L-J) * K;
		    L = (L-K) * (K+J);
		    E1[L-1] = J + K + L;
		    E1[K-1] = J * K * L;
		}

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N6,J,K,E1[1],E1[2],E1[3],E1[4]);
	#endif

	/*
	C
	C	Module 7: Trigonometric functions
	C
	*/
		X = 0.5;
		Y = 0.5;

		for (I = 1; I <= N7; I++) {
			X = T * DATAN(T2*DSIN(X)*DCOS(X)/(DCOS(X+Y)+DCOS(X-Y)-1.0));
			Y = T * DATAN(T2*DSIN(Y)*DCOS(Y)/(DCOS(X+Y)+DCOS(X-Y)-1.0));
		}

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N7,J,K,X,X,Y,Y);
	#endif

	/*
	C
	C	Module 8: Procedure calls
	C
	*/
		X = 1.0;
		Y = 1.0;
		Z = 1.0;

		for (I = 1; I <= N8; I++)
			P3(X,Y,&Z);

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N8,J,K,X,Y,Z,Z);
	#endif

	/*
	C
	C	Module 9: Array references
	C
	*/
		J = 1;
		K = 2;
		L = 3;
		E1[1] = 1.0;
		E1[2] = 2.0;
		E1[3] = 3.0;

		for (I = 1; I <= N9; I++)
			P0();

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N9,J,K,E1[1],E1[2],E1[3],E1[4]);
	#endif

	/*
	C
	C	Module 10: Integer arithmetic
	C
	*/
		J = 2;
		K = 3;

		for (I = 1; I <= N10; I++) {
		    J = J + K;
		    K = J + K;
		    J = K - J;
		    K = K - J - J;
		}

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N10,J,K,X1,X2,X3,X4);
	#endif

	/*
	C
	C	Module 11: Standard functions
	C
	*/
		X = 0.75;

		for (I = 1; I <= N11; I++)
			X = DSQRT(DEXP(DLOG(X)/T1));

	#ifdef PRINTOUT
		IF (JJ==II)POUT(N11,J,K,X,X,X,X);
	#endif

	/*
	C
	C      THIS IS THE END OF THE MAJOR LOOP.
	C
	*/
		if (++JJ <= II)
			goto IILOOP;

	/*
	C
	C      Stop benchmark timing at this point.
	C
	*/
		fini_tick = HAL_GetTick();

	/*
	C----------------------------------------------------------------
	C      Performance in Whetstone KIP's per second is given by
	C
	C	(100*LOOP*II)/TIME
	C
	C      where TIME is in seconds.
	C--------------------------------------------------------------------
	*/
		printf("\n");
		if (fini_tick-start_tick <= SYSTIME_CLOCK) {
			printf("Insufficient duration- Increase the LOOP count\n");
			return(1);
		}

		printf("Loops: %ld, Iterations: %d, Duration: %u/%u sec.\n",
				LOOP, II, fini_tick-start_tick, SYSTIME_CLOCK);

		KIPS = (100.0*LOOP*II)/(float)((fini_tick-start_tick)/SYSTIME_CLOCK);
		if (KIPS >= 1000.0)
			printf("C Converted Double Precision Whetstones: %.1f MIPS\n", KIPS/1000.0);
		else
			printf("C Converted Double Precision Whetstones: %.1f KIPS\n", KIPS);

		printf("%d %u\n",
				_mfcr(CPU_CORE_ID),
				g_SysTicks);

		if (continuous)
			goto LCONT;

	while(1)
	{
		if(0!=test_flag)
		{
			test_flag = 0;
			printf("%d %u\n",
					_mfcr(CPU_CORE_ID),
					g_SysTicks);
		}
	}
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
					my_puts(my_str);
					break;
				case 'E' :
					quit = 1;
					my_puts("Bye bye!");
					break;
				case '\n' :
				case '\r' :
					/* do nothing -- ignore it */
					break;
				default :
					my_printf("Command '%c' not supported\r\n", c);
					break;
			}
		}
	}

	/* wait until sending has finished */
	while (_uart_sending())
		;

	return EXIT_SUCCESS;
}

