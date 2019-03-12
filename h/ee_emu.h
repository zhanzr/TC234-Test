/*
 * ee_emu.h
 *
 *  Created on: Mar 11, 2019
 *      Author: zzr
 */

#ifndef EE_EMU_H_
#define EE_EMU_H_

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
#include "data_flash.h"
#include "uart_int.h"

//sector 1 and sector 2 are used
#define	DFLASH0_START	0xAF000000
#define DFLASH_SECTOR_SIZE	(8*1024)
#define SECTOR_SIZE               DFLASH_SECTOR_SIZE

#define EMU_EE_ADDR_START	0
#define EMU_EE_ADDR_END	63
#define EMU_EE_ADDR_L	(EMU_EE_ADDR_END-EMU_EE_ADDR_START+1)

/* EEPROM emulation start address in Flash */
/* From sector1 : after 2 sectors of used Flash memory */
#define EEPROM_START_ADDRESS  ((uint32_t)DFLASH0_START+SECTOR_SIZE)

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (SECTOR_SIZE - 1)))

#define PAGE0_ID               1

#define PAGE1_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + SECTOR_SIZE))
#define PAGE1_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (2 * SECTOR_SIZE - 1)))

#define PAGE1_ID               2

#define	NO_VALID_PAGE	0x00AB

/* Page status definitions */
#define ERASED                ((uint64_t)0x0000000000000000)     /* Page is empty */
#define RECEIVE_DATA          ((uint64_t)0xEEEEEEEEEEEEEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint64_t)0xFFFFFFFFFFFFFFFF)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE   ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL             ((uint8_t)0x80)

/* Variables' number */
#define NB_OF_VAR             ((uint8_t)EMU_EE_ADDR_L)

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t EE_Init(void);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint32_t Data);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint32_t* Data);

#endif /* EE_EMU_H_ */
