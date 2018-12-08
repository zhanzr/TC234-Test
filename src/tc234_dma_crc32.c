/*****************************************************
 *
 * DMA_CRC32.c
 *
 * Description :    This program uses the AURIX DMA peripheral to calculate the CRC-32 over a range of memory.
 *                  - DMA linked lists minimize CPU overhead.
 *                  - Block transfers (BLKM) are used to handle over 512K in a single transaction.
 *                  - The second transaction handles the remainder memory range.
 *                  - The DMA completion service request is routed to a CPU interrupt.
 
 The TC234 doesn't have the Flexible CRC Engine, but it does have the CRC32 instruction, and the CRC32 used by the DMA engine. Here's some sample code that calculates the CRC32 of 0xA0018000 through 0xA009FFBF, first using the CRC32 instruction, and then using DMA.

 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <machine/intrinsics.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxEth_reg.h)
#include TC_INCLUDE(TCPATH/IfxEth_bf.h)
#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxInt_reg.h)
#include TC_INCLUDE(TCPATH/IfxInt_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)
#include TC_INCLUDE(TCPATH/IfxQspi_reg.h)
#include TC_INCLUDE(TCPATH/IfxQspi_bf.h)
#include TC_INCLUDE(TCPATH/IfxDma_reg.h)
#include TC_INCLUDE(TCPATH/IfxDma_bf.h)

#include "core_tc23x.h"
#include "system_tc2x.h"
#include "cint_trap_tc23x.h"
#include "interrupts_tc23x.h"
#include "led.h"
#include "uart_int.h"
#include "asm_prototype.h"
#include "dts.h"
#include "timer.h"
#include "gpsr.h"
#include "scu_eru.h"
#include "data_flash.h"

#define DMA_CHANNEL_CRC32    0
#define ISR_PRIO_DMA_TEST	30

const Ifx_DMA *dmaSFR = &MODULE_DMA;

extern const Ifx_DMA_CH CRC32_SCAN;
extern const Ifx_DMA_CH CRC32_SCAN_A009FFC0;
extern const Ifx_DMA_CH CRC32_DONE;
extern const Ifx_DMA_CH CRC32_FINISH;

volatile uint32_t dummy_word;

volatile uint32_t dma_done;

// *** Scan starts at 0xA0018000 ***
// Do as much as possible in one DMA move, and then handle the remainder in a second move
//    move 0x10 times per transfer
//    times 4 bytes each
//    times 0x21FF transfers
// 0x10 * 4 * 0x21FF = 0x87FC0 bytes
// A0018000, fill 0x87FC0 bytes = A0018000-A009FFBF

const Ifx_DMA_CH CRC32_SCAN __attribute__ ((aligned(32))) =
{
    .SADR.U = (uint32_t) 0xA0018000,         // read application memory
    .DADR.U = (uint32_t) &dummy_word,         // write into dummy word

    .ADICR.B.SHCT = 0xD,    // DMA accumulated linked list
    .ADICR.B.INCS = 1,      // Increment source after each transfer
    .ADICR.B.DCBE = 1,      // Enable destination circular buffer (won't change destination since CBLD=0)

    .CHCFGR.B.TREL = 0x21FF,// Transfer reload
    .CHCFGR.B.CHDW = 2U,    // 32-bit transfer
    .CHCFGR.B.BLKM = 4,     // each transfer has 16 DMA moves
    .CHCFGR.B.RROAT = 1,    // Reset TSRz after DMA transaction (not single transfer)

    .CHCSR.B.SCH = 1,       // Set transaction request to start immediately

    .SHADR.U = (uint32_t) &CRC32_SCAN_A009FFC0,
};

// Handle the remainder in a second DMA move

const Ifx_DMA_CH CRC32_SCAN_A009FFC0 __attribute__ ((aligned(32))) =
{
    .SADR.U = (uint32_t) 0xA009FFC0,         // read application memory
    .DADR.U = (uint32_t) &dummy_word,         // write into dummy word

    .ADICR.B.SHCT = 0xD,    // DMA accumulated linked list
    .ADICR.B.INCS = 1,      // Increment source after each transfer
    .ADICR.B.DCBE = 1,      // Enable destination circular buffer (won't change destination since CBLD=0)
    .ADICR.B.INTCT = 2,     // raise service request when finished

    .CHCFGR.B.TREL = 15,    // Transfer reload
    .CHCFGR.B.CHDW = 2U,    // 32-bit transfer
    .CHCFGR.B.RROAT = 1,    // Reset TSRz after DMA transaction (not single transfer)

    .CHCSR.B.SCH = 1,       // Set transaction request to start immediately

    .SHADR.U = (uint32_t) &CRC32_FINISH,
};

const Ifx_DMA_CH CRC32_FINISH __attribute__ ((aligned(32))) =
{
    .SADR.U = (uint32_t) NULL,
    .DADR.U = (uint32_t) NULL,

    .ADICR.B.SHCT = 0xD,    // DMA accumulated linked list
    .ADICR.B.INCS = 1,      // Increment source after each transfer
    .ADICR.B.DCBE = 1,      // Enable destination circular buffer (won't change destination since CBLD=0)

    .CHCFGR.B.TREL = 0,
    .CHCFGR.B.CHDW = 2U,    // 32-bit transfer

    .SHADR.U = (uint32_t) NULL,
    .CHCSR.B.SCH = 0,       // TRANSACTION DOES NOT START IMMEDIATELY - end of list
};

static void DMA_Start( void );

void test_dma_crc(void)
{
	_enable();

    DMA_Start();
}

void dma_isr( void )
{
    // User code here!
    dma_done = 1;
}

static void DMA_Start( void )
{
    volatile unsigned int dummy;

	unlock_wdtcon();
    DMA_CLC.U = 0x0000;     // Enable module clock and ctrl.
    dummy = DMA_CLC.U;      // Read back ensures buffers are flushed
//    SRC_DMACH0.B.SRPN = ISR_PRIO_DMA_TEST;   // Route VADC service request line 0 to ISR priority
//    SRC_DMACH0.B.TOS = 0;    // service control: CPU
//    SRC_DMACH0.B.SRE = 1;    // service request enable
	lock_wdtcon();

	InterruptInstall(SRC_ID_DMACH0, dma_isr, ISR_PRIO_DMA_TEST, 0);

    // Calculate the CRC32 with the CRC32 instruction (all CPU)
//    {
//        uint32_t i;
//        uint32_t *src;
//        uint32_t crc=0;
//        src = (uint32_t *) 0xA0018000;
//        for( i=0; i<556992/4; i++ )
//        {
//            crc = __crc32(crc,*src++);
//        }
//        P10_OUT.B.P6 = (crc & 1);
//    }

    // Calculate the CRC32 using DMA
	// - Set up DMA channel 0
    {
        uint32_t i;
        uint32_t *src, *dst;

        src = (uint32_t *)&CRC32_SCAN;
        dst = (uint32_t *)&MODULE_DMA.CH[DMA_CHANNEL_CRC32];

        for (i=0; i<(sizeof(Ifx_DMA_CH)/4); i++)
        {
        	*dst++ = *src++;
        }
    }

    printf("Wait for CRC32 DMA ...\n");
	flush_stdout();
    // You could go off and do something more useful instead of just polling for completion
    // - dma_done is set by the DMA channel interrupt service routine
    //
    while( dma_done == 0 )
    {
    	_nop();
    }

    printf("CRC32 DMA Done.\n");
	flush_stdout();
}
