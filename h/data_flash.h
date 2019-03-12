/*
 * data_flash.h
 *
 *  Created on: Nov 18, 2018
 *      Author: zzr
 */

#ifndef DATA_FLASH_H_
#define DATA_FLASH_H_

#include <stdbool.h>
#include <stdint.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxFlash_reg.h)
#include TC_INCLUDE(TCPATH/IfxFlash_bf.h)
#include TC_INCLUDE(TCPATH/IfxPmu_reg.h)
#include TC_INCLUDE(TCPATH/IfxPmu_bf.h)

#include "interrupts_tc23x.h"
#include "core_tc23x.h"
#include "asm_prototype.h"

/** \brief base address for general command sequences
 */

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/** \brief base address for general command sequences
 */
#define IFXFLASH_CMD_BASE_ADDRESS (0xaf000000)
#define DFLASH_SECTOR_SIZE	(8*1024)

/** \brief number of flash modules
 */
#define IFXFLASH_NUM_FLASH_MODULES (1)

#define IFXFLASH_PFLASH_BANKS (2)

/** \brief
 */
#define IFXFLASH_PFLASH_NUM_LOG_SECTORS (IFXFLASH_PFLASH_BANKS * 27)

#define IFXFLASH_PFLASH_NUM_PHYSICAL_SECTORS (IFXFLASH_PFLASH_BANKS*4)

#define IFXFLASH_PFLASH_PAGE_LENGTH (32)

/** \brief Phy sector for DF
 */
#define IFXFLASH_DFLASH_NUM_PHYSICAL_SECTORS (1)

#define IFXFLASH_DFLASH_NUM_LOG_SECTORS (16)

#define IFXFLASH_DFLASH_START (0xaf000000)

#define IFXFLASH_DFLASH_SIZE (IFXFLASH_DFLASH_NUM_LOG_SECTORS*0x2000)

#define IFXFLASH_DFLASH_END (IFXFLASH_DFLASH_START+IFXFLASH_DFLASH_SIZE-1)

#define IFXFLASH_DFLASH_NUM_HSM_LOG_SECTORS (8)

#define IFXFLASH_DFLASH_NUM_UCB_LOG_SECTORS (16)

#define IFXFLASH_DFLASH_BANKS (1)

#define IFXFLASH_DFLASH_PAGE_LENGTH (8)

#define IFXFLASH_ERROR_TRACKING_MAX_CORRECTABLE_ERRORS (10)

#define IFXFLASH_ERROR_TRACKING_MAX_UNCORRECTABLE_ERRORS (1)

/** \brief offset between PMU PFlash ranges
 */
#define IFXFLASH_PFLASH_OFFSET (0x00800000)

/** \brief offset between command areas (in DFlash range)
 */
#define IFXFLASH_CMD_BASE_OFFSET (0x00100000)

/** \brief P flash burst length
 */
#define IFXFLASH_PFLASH_BURST_LENGTH (0x100)

/** \brief Dflash burst length
 */
#define IFXFLASH_DFLASH_BURST_LENGTH (0x20)

/** \brief p flash start
 */
#define IFXFLASH_PFLASH_START (0xa0000000)

/** \brief p flash size
 */
#define IFXFLASH_PFLASH_SIZE (0x00400000  )

/** \brief p flash end
 */
#define IFXFLASH_PFLASH_END (IFXFLASH_PFLASH_START + IFXFLASH_PFLASH_SIZE-1)

typedef enum {
    IfxFlash_FlashType_Fa = 0,  /**< \brief Flash Array */
    IfxFlash_FlashType_D0 = 1,  /**< \brief data flash #0 */
    IfxFlash_FlashType_D1 = 2,  /**< \brief data flash #1 */
    IfxFlash_FlashType_P0 = 3,  /**< \brief program flash #0 */
    IfxFlash_FlashType_P1 = 4,  /**< \brief program flash #1 */
    IfxFlash_FlashType_P2 = 5,  /**< \brief program flash #2 */
    IfxFlash_FlashType_P3 = 6   /**< \brief program flash #3 */
} IfxFlash_FlashType;

/** \brief contains start and end address of sectors
*/
typedef struct {
    uint32_t    start;  /**< \brief start address of sector */
    uint32_t    end;    /**< \brief end address of sector */
} IfxFlash_flashSector;

void config_dflash(void);

void test_proc_dflash(void);

#endif /* DATA_FLASH_H_ */
