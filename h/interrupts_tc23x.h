/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Handling of interrupts on TC23x
*
* Copyright HighTec EDV-Systeme GmbH 1982-2015
*====================================================================*/

#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

/* Interrupt SRC IDs */
/* taken from Table 16-4 (TC23x/TC22x UM v1.0) */
#define SRC_ID_CPU0SBSRC		0	/* CPU 0 Software Breakpoint Service Request */

#define SRC_ID_EMEM				8	/* Emulation Memory Service Request (ED only) */

#define SRC_ID_BCUSPBSRC		16	/* Bus Control Unit SPB Service Request */

#define SRC_ID_XBARSRC			18	/* XBAR_SRI Service Request */

#define SRC_ID_CERBERUS0		20	/* Cerberus Service Request 0 */
#define SRC_ID_CERBERUS1		21	/* Cerberus Service Request 1 */

#define SRC_ID_ASCLIN0TX		32	/* ASCLIN 0 Transmit Service Request */
#define SRC_ID_ASCLIN0RX		33	/* ASCLIN 0 Receive Service Request */
#define SRC_ID_ASCLIN0EX		34	/* ASCLIN 0 Error Service Request */
#define SRC_ID_ASCLIN1TX		35	/* ASCLIN 1 Transmit Service Request */
#define SRC_ID_ASCLIN1RX		36	/* ASCLIN 1 Receive Service Request */
#define SRC_ID_ASCLIN1EX		37	/* ASCLIN 1 Error Service Request */

#define SRC_ID_QSPI0TX			100	/* QSPI 0 Transmit Service Request */
#define SRC_ID_QSPI0RX			101	/* QSPI 0 Receive Service Request */
#define SRC_ID_QSPI0ERR			102	/* QSPI 0 Error Service Request */
#define SRC_ID_QSPI0PT			103	/* QSPI 0 Phase Transition Service Request */
#define SRC_ID_QSPI0HC			104	/* QSPI 0 High Speed Capture Service Request (Reserved) */
#define SRC_ID_QSPI0U			105	/* QSPI 0 User Defined Service Request */
#define SRC_ID_QSPI1TX			106	/* QSPI 1 Transmit Service Request */
#define SRC_ID_QSPI1RX			107	/* QSPI 1 Receive Service Request */
#define SRC_ID_QSPI1ERR			108	/* QSPI 1 Error Service Request */
#define SRC_ID_QSPI1PT			109	/* QSPI 1 Phase Transition Service Request */
#define SRC_ID_QSPI1HC			110	/* QSPI 1 High Speed Capture Service Request (Reserved) */
#define SRC_ID_QSPI1U			111	/* QSPI 1 User Defined Service Request */
#define SRC_ID_QSPI2TX			112	/* QSPI 2 Transmit Service Request */
#define SRC_ID_QSPI2RX			113	/* QSPI 2 Receive Service Request */
#define SRC_ID_QSPI2ERR			114	/* QSPI 2 Error Service Request */
#define SRC_ID_QSPI2PT			115	/* QSPI 2 Phase Transition Service Request */
#define SRC_ID_QSPI2HC			116	/* QSPI 2 High Speed Capture Service Request */
#define SRC_ID_QSPI2U			117	/* QSPI 2 User Defined Service Request */
#define SRC_ID_QSPI3TX			118	/* QSPI 3 Transmit Service Request */
#define SRC_ID_QSPI3RX			119	/* QSPI 3 Receive Service Request */
#define SRC_ID_QSPI3ERR			120	/* QSPI 3 Error Service Request */
#define SRC_ID_QSPI3PT			121	/* QSPI 3 Phase Transition Service Request */
#define SRC_ID_QSPI3HC			122	/* QSPI 3 High Speed Capture Service Request */
#define SRC_ID_QSPI3U			123	/* QSPI 3 User Defined Service Request */

#define SRC_ID_SENT0			212	/* SENT TRIG0 Service Request */
#define SRC_ID_SENT1			213	/* SENT TRIG1 Service Request */
#define SRC_ID_SENT2			214	/* SENT TRIG2 Service Request */
#define SRC_ID_SENT3			215	/* SENT TRIG3 Service Request */

#define SRC_ID_CCU60SR0			264	/* CCU6 0 Service Request 0 */
#define SRC_ID_CCU60SR1			265	/* CCU6 0 Service Request 1 */
#define SRC_ID_CCU60SR2			266	/* CCU6 0 Service Request 2 */
#define SRC_ID_CCU60SR3			267	/* CCU6 0 Service Request 3 */
#define SRC_ID_CCU61SR0			268	/* CCU6 1 Service Request 0 */
#define SRC_ID_CCU61SR1			269	/* CCU6 1 Service Request 1 */
#define SRC_ID_CCU61SR2			270	/* CCU6 1 Service Request 2 */
#define SRC_ID_CCU61SR3			271	/* CCU6 1 Service Request 3 */

#define SRC_ID_GPT120CIRQ		280	/* GPT120 CAPREL Service Request */
#define SRC_ID_GPT120T2			281	/* GPT120 T2 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T3			282	/* GPT120 T3 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T4			283	/* GPT120 T4 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T5			284	/* GPT120 T5 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T6			285	/* GPT120 T6 Overflow/Underflow Service Request */

#define SRC_ID_STM0SR0			292	/* System Timer 0 Service Request 0 */
#define SRC_ID_STM0SR1			293	/* System Timer 0 Service Request 1 */

#define SRC_ID_FCE				300	/* FCE Error Service Request */

#define SRC_ID_DMAERR			316	/* DMA Error Service Request */

#define SRC_ID_DMACH0			320	/* DMA Channel  0 Service Request */
#define SRC_ID_DMACH1			321	/* DMA Channel  1 Service Request */
#define SRC_ID_DMACH2			322	/* DMA Channel  2 Service Request */
#define SRC_ID_DMACH3			323	/* DMA Channel  3 Service Request */
#define SRC_ID_DMACH4			324	/* DMA Channel  4 Service Request */
#define SRC_ID_DMACH5			325	/* DMA Channel  5 Service Request */
#define SRC_ID_DMACH6			326	/* DMA Channel  6 Service Request */
#define SRC_ID_DMACH7			327	/* DMA Channel  7 Service Request */
#define SRC_ID_DMACH8			328	/* DMA Channel  8 Service Request */
#define SRC_ID_DMACH9			329	/* DMA Channel  9 Service Request */
#define SRC_ID_DMACH10			330	/* DMA Channel 10 Service Request */
#define SRC_ID_DMACH11			331	/* DMA Channel 11 Service Request */
#define SRC_ID_DMACH12			332	/* DMA Channel 12 Service Request */
#define SRC_ID_DMACH13			333	/* DMA Channel 13 Service Request */
#define SRC_ID_DMACH14			334	/* DMA Channel 14 Service Request */
#define SRC_ID_DMACH15			335	/* DMA Channel 15 Service Request */

#define SRC_ID_ETH				572	/* Ethernet Service Request */

#define SRC_ID_CANINT0			576	/* MultiCAN Service Request 0 */
#define SRC_ID_CANINT1			577	/* MultiCAN Service Request 1 */
#define SRC_ID_CANINT2			578	/* MultiCAN Service Request 2 */
#define SRC_ID_CANINT3			579	/* MultiCAN Service Request 3 */
#define SRC_ID_CANINT4			580	/* MultiCAN Service Request 4 */
#define SRC_ID_CANINT5			581	/* MultiCAN Service Request 5 */
#define SRC_ID_CANINT6			582	/* MultiCAN Service Request 6 */
#define SRC_ID_CANINT7			583	/* MultiCAN Service Request 7 */
#define SRC_ID_CANINT8			584	/* MultiCAN Service Request 8 */
#define SRC_ID_CANINT9			585	/* MultiCAN Service Request 9 */
#define SRC_ID_CANINT10			586	/* MultiCAN Service Request 10 */
#define SRC_ID_CANINT11			587	/* MultiCAN Service Request 11 */
#define SRC_ID_CANINT12			588	/* MultiCAN Service Request 12 */
#define SRC_ID_CANINT13			589	/* MultiCAN Service Request 13 */
#define SRC_ID_CANINT14			590	/* MultiCAN Service Request 14 */
#define SRC_ID_CANINT15			591	/* MultiCAN Service Request 15 */
#define SRC_ID_CAN1INT0			592	/* MultiCAN1 Service Request 0 */
#define SRC_ID_CAN1INT1			593	/* MultiCAN1 Service Request 1 */
#define SRC_ID_CAN1INT2			594	/* MultiCAN1 Service Request 2 */
#define SRC_ID_CAN1INT3			595	/* MultiCAN1 Service Request 3 */
#define SRC_ID_CAN1INT4			596	/* MultiCAN1 Service Request 4 */
#define SRC_ID_CAN1INT5			597	/* MultiCAN1 Service Request 5 */
#define SRC_ID_CAN1INT6			598	/* MultiCAN1 Service Request 6 */
#define SRC_ID_CAN1INT7			599	/* MultiCAN1 Service Request 7 */

#define SRC_ID_VADCG0SR0		608	/* VADC Group 0 Service Request 0 */
#define SRC_ID_VADCG0SR1		609	/* VADC Group 0 Service Request 1 */
#define SRC_ID_VADCG0SR2		610	/* VADC Group 0 Service Request 2 */
#define SRC_ID_VADCG0SR3		611	/* VADC Group 0 Service Request 3 */
#define SRC_ID_VADCG1SR0		612	/* VADC Group 1 Service Request 0 */
#define SRC_ID_VADCG1SR1		613	/* VADC Group 1 Service Request 1 */
#define SRC_ID_VADCG1SR2		614	/* VADC Group 1 Service Request 2 */
#define SRC_ID_VADCG1SR3		615	/* VADC Group 1 Service Request 3 */
#define SRC_ID_VADCG2SR0		616	/* VADC Group 2 Service Request 0 */
#define SRC_ID_VADCG2SR1		617	/* VADC Group 2 Service Request 1 */
#define SRC_ID_VADCG2SR2		618	/* VADC Group 2 Service Request 2 */
#define SRC_ID_VADCG2SR3		619	/* VADC Group 2 Service Request 3 */
#define SRC_ID_VADCG3SR0		620	/* VADC Group 3 Service Request 0 */
#define SRC_ID_VADCG3SR1		621	/* VADC Group 3 Service Request 1 */
#define SRC_ID_VADCG3SR2		622	/* VADC Group 3 Service Request 2 */
#define SRC_ID_VADCG3SR3		623	/* VADC Group 3 Service Request 3 */

#define SRC_ID_VADCCG0SR0		680	/* VADC Common Group 0 Service Request 0 */
#define SRC_ID_VADCCG0SR1		681	/* VADC Common Group 0 Service Request 1 */
#define SRC_ID_VADCCG0SR2		682	/* VADC Common Group 0 Service Request 2 */
#define SRC_ID_VADCCG0SR3		683	/* VADC Common Group 0 Service Request 3 */

#define SRC_ID_ERAYINT0			760	/* E-RAY Service Request 0 */
#define SRC_ID_ERAYINT1			761	/* E-RAY Service Request 1 */
#define SRC_ID_ERAYTINT0		762	/* E-RAY Timer Interrupt 0 Service Request */
#define SRC_ID_ERAYTINT1		763	/* E-RAY Timer Interrupt 1 Service Request */
#define SRC_ID_ERAYNDAT0		764	/* E-RAY New Data 0 Service Request */
#define SRC_ID_ERAYNDAT1		765	/* E-RAY New Data 1 Service Request */
#define SRC_ID_ERAYMBSC0		766	/* E-RAY Message Buffer Status Changed 0 Service Request */
#define SRC_ID_ERAYMBSC1		767	/* E-RAY Message Buffer Status Changed 1 Service Request */
#define SRC_ID_ERAYOBUSY		768	/* E-RAY Output Buffer Busy Service Request */
#define SRC_ID_ERAYIBUSY		769	/* E-RAY Input Buffer Busy Service Request */

#define SRC_ID_PMU00			780	/* PMU 0 Service Request 0 */
#define SRC_ID_PMU01			781	/* PMU 0 Service Request 1 */

#define SRC_ID_HSM0				816	/* HSM Service Request 0 */
#define SRC_ID_HSM1				817	/* HSM Service Request 1 */

#define SRC_ID_SCUDTS			820	/* SCU DTS Busy Service Request */
#define SRC_ID_SCUERU0			821	/* SCU ERU Service Request 0 */
#define SRC_ID_SCUERU1			822	/* SCU ERU Service Request 1 */
#define SRC_ID_SCUERU2			823	/* SCU ERU Service Request 2 */
#define SRC_ID_SCUERU3			824	/* SCU ERU Service Request 3 */

#define SRC_ID_SMU0				836	/* SMU Service Request 0 */
#define SRC_ID_SMU1				837	/* SMU Service Request 1 */
#define SRC_ID_SMU2				838	/* SMU Service Request 2 */

#define SRC_ID_LMU				888	/* LMU Error Service Request */

#define SRC_ID_EVRWUT			1004	/* EVR Wake Up Timer Service Request */
#define SRC_ID_SCDC				1005	/* EVR Service Request */

#define SRC_ID_FFTDONE			1008	/* FFT Done Service Request */
#define SRC_ID_FFTERR			1009	/* FFT Error Service Request */
#define SRC_ID_FFTRFS			1010	/* FFT Ready For Start Service Request */

#define SRC_ID_GPSR00			1024	/* General Purpose Service Request 0 0 */
#define SRC_ID_GPSR01			1025	/* General Purpose Service Request 0 1 */
#define SRC_ID_GPSR02			1026	/* General Purpose Service Request 0 2 */
#define SRC_ID_GPSR03			1027	/* General Purpose Service Request 0 3 */

#define SRC_ID_GTMAEIIRQ		1408	/* GTM AEI Shared Service Request */

#define SRC_ID_GTMERR			1500	/* GTM Error Service Request */

#define SRC_ID_GTMTIM00			1504	/* GTM TIM0 Shared Service Request 0 */
#define SRC_ID_GTMTIM01			1505	/* GTM TIM0 Shared Service Request 1 */
#define SRC_ID_GTMTIM02			1506	/* GTM TIM0 Shared Service Request 2 */
#define SRC_ID_GTMTIM03			1507	/* GTM TIM0 Shared Service Request 3 */
#define SRC_ID_GTMTIM04			1508	/* GTM TIM0 Shared Service Request 4 */
#define SRC_ID_GTMTIM05			1509	/* GTM TIM0 Shared Service Request 5 */
#define SRC_ID_GTMTIM06			1510	/* GTM TIM0 Shared Service Request 6 */
#define SRC_ID_GTMTIM07			1511	/* GTM TIM0 Shared Service Request 7 */

#define SRC_ID_GTMTOM00			1760	/* GTM TOM0 Shared Service Request 0 */
#define SRC_ID_GTMTOM01			1761	/* GTM TOM0 Shared Service Request 1 */
#define SRC_ID_GTMTOM02			1762	/* GTM TOM0 Shared Service Request 2 */
#define SRC_ID_GTMTOM03			1763	/* GTM TOM0 Shared Service Request 3 */
#define SRC_ID_GTMTOM04			1764	/* GTM TOM0 Shared Service Request 4 */
#define SRC_ID_GTMTOM05			1765	/* GTM TOM0 Shared Service Request 5 */
#define SRC_ID_GTMTOM06			1766	/* GTM TOM0 Shared Service Request 6 */
#define SRC_ID_GTMTOM07			1767	/* GTM TOM0 Shared Service Request 7 */
#define SRC_ID_GTMTOM10			1768	/* GTM TOM1 Shared Service Request 0 */
#define SRC_ID_GTMTOM11			1769	/* GTM TOM1 Shared Service Request 1 */
#define SRC_ID_GTMTOM12			1770	/* GTM TOM1 Shared Service Request 2 */
#define SRC_ID_GTMTOM13			1771	/* GTM TOM1 Shared Service Request 3 */
#define SRC_ID_GTMTOM14			1772	/* GTM TOM1 Shared Service Request 4 */
#define SRC_ID_GTMTOM15			1773	/* GTM TOM1 Shared Service Request 5 */
#define SRC_ID_GTMTOM16			1774	/* GTM TOM1 Shared Service Request 6 */
#define SRC_ID_GTMTOM17			1775	/* GTM TOM1 Shared Service Request 7 */

#define IRQ_ID_MAX_NUM			1776


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//User Priority Definition
#define GPSR0_ISR_PRIO		1
#define STM_CMP0_ISR_PRIO	2
#define STM_CMP1_ISR_PRIO	3

#define GPSR0_ISR_PRIO	12
#define GPSR1_ISR_PRIO	13
#define GPSR2_ISR_PRIO	14
#define GPSR3_ISR_PRIO	15
#define DTS_ISR_PRIO	16

#define ERS0_ISR_PRIO	17
#define ERS1_ISR_PRIO	18
#define ERS2_ISR_PRIO	19
#define ERS3_ISR_PRIO	20

#define TXC_ISR_PRIO			21
#define RXD_ISR_PRIO			22

/* type of an Interrupt Service Routine (ISR) */
typedef void (*isrhnd_t)(int arg);


/*---------------------------------------------------------------------
	Function:	InterruptInit
	Purpose:	Initialisation of interrupt handling
	Arguments:	void
	Return:		void
---------------------------------------------------------------------*/
void InterruptInit(void);

/*---------------------------------------------------------------------
	Function:	InterruptInstall
	Purpose:	Install a service handler for an interrupt
	Arguments:	int irqNum       - number of interrupt
				isrhnd_t isrProc - pointer to service routine
				int prio         - priority (1-255)
				int arg          - argument for service routine
	Return:		void
---------------------------------------------------------------------*/
void InterruptInstall(int irqNum, isrhnd_t isrProc, int prio, int arg);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INTERRUPTS_H__ */
