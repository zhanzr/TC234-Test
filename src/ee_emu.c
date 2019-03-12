/*
 * ee_emu.c
 *
 *  Created on: Mar 11, 2019
 *      Author: zzr
 */


/** @addtogroup EEPROM_Emulation
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "ee_emu.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */
uint32_t DataVar = 0;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static uint32_t EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint32_t VirtAddress, uint32_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint32_t Data);
static uint16_t EE_VerifyPageFullyErased(uint32_t Address);

const IfxFlash_flashSector IfxFlash_dFlashTableEepLog[] = {
		{0xAF000000 + 0 * 0x2000,  0xAF000000 + 1 * 0x2000 - 1 },
		{0xAF000000 + 1 * 0x2000,  0xAF000000 + 2 * 0x2000 - 1 },
		{0xAF000000 + 2 * 0x2000,  0xAF000000 + 3 * 0x2000 - 1 },
		{0xAF000000 + 3 * 0x2000,  0xAF000000 + 4 * 0x2000 - 1 },
		{0xAF000000 + 4 * 0x2000,  0xAF000000 + 5 * 0x2000 - 1 },
		{0xAF000000 + 5 * 0x2000,  0xAF000000 + 6 * 0x2000 - 1 },
		{0xAF000000 + 6 * 0x2000,  0xAF000000 + 7 * 0x2000 - 1 },
		{0xAF000000 + 7 * 0x2000,  0xAF000000 + 8 * 0x2000 - 1 },
		{0xAF000000 + 8 * 0x2000,  0xAF000000 + 9 * 0x2000 - 1 },
		{0xAF000000 + 9 * 0x2000,  0xAF000000 + 10 * 0x2000 - 1},
		{0xAF000000 + 10 * 0x2000, 0xAF000000 + 11 * 0x2000 - 1},
		{0xAF000000 + 11 * 0x2000, 0xAF000000 + 12 * 0x2000 - 1},
		{0xAF000000 + 12 * 0x2000, 0xAF000000 + 13 * 0x2000 - 1},
		{0xAF000000 + 13 * 0x2000, 0xAF000000 + 14 * 0x2000 - 1},
		{0xAF000000 + 14 * 0x2000, 0xAF000000 + 15 * 0x2000 - 1},
		{0xAF000000 + 15 * 0x2000, 0xAF000000 + 16 * 0x2000 - 1},
};

const IfxFlash_flashSector IfxFlash_dFlashTableHsmLog[IFXFLASH_DFLASH_NUM_HSM_LOG_SECTORS] = {
		{0xaf110000, 0xaf111fff},   // HSM0
		{0xaf112000, 0xaf113fff},   // HSM1
		{0xaf114000, 0xaf115fff},   // HSM2
		{0xaf116000, 0xaf117fff},   // HSM3
		{0xaf118000, 0xaf119fff},   // HSM4
		{0xaf11a000, 0xaf11bfff},   // HSM5
		{0xaf11c000, 0xaf11dfff},   // HSM6
		{0xaf11e000, 0xaf11ffff},   // HSM7
};

const IfxFlash_flashSector IfxFlash_dFlashTablePhys[IFXFLASH_DFLASH_NUM_PHYSICAL_SECTORS] = {
		{IFXFLASH_DFLASH_START, IFXFLASH_DFLASH_END},
};

void IfxFlash_eraseSector(uint32_t sectorAddr)
{
	uint32_t *addr1 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa50);
	uint32_t *addr2 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa58);
	uint32_t *addr3 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);
	uint32_t *addr4 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);

	*addr1 = sectorAddr;
	*addr2 = 1;
	*addr3 = 0x80;
	*addr4 = 0x50;

	_dsync();
}

void IfxFlash_loadPage2X32(uint32_t pageAddr, uint32_t wordL, uint32_t wordU){
	uint32_t *addr1 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0x55f0);

	*addr1 = wordL;
	addr1++;
	*addr1 = wordU;

	_dsync();
}

uint8_t IfxFlash_waitUnbusy(uint32_t flash, IfxFlash_FlashType flashType)
{
	if (flash == 0)
	{
		while (FLASH0_FSR.U & (1 << flashType))
		{}
	}

#if IFXFLASH_NUM_FLASH_MODULES > 1
	else if (flash == 1)
	{
		while (FLASH1_FSR.U & (1 << flashType))
		{}
	}
#endif
	else
	{
		return 1; // invalid flash selected
	}
	_dsync();
	return 0;     // finished
}

uint8_t IfxFlash_enterPageMode(uint32_t pageAddr)
{
	uint32_t *addr1 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0x5554);

	if ((pageAddr & 0xff000000) == 0xa0000000)    // program flash
	{
		*addr1 = 0x50;
		return 0;
	}
	else if ((pageAddr & 0xff000000) == 0xaf000000)       // data flash
	{
		*addr1 = 0x5D;
		return 0;
	}

	_dsync();
	return 1; // invalid flash address
}


void IfxFlash_writePage(uint32_t pageAddr)
{
	uint32_t *addr1 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa50);
	uint32_t *addr2 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa58);
	uint32_t *addr3 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);
	uint32_t *addr4 = (uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);

	*addr1 = pageAddr;
	*addr2 = 0x00;
	*addr3 = 0xa0;
	*addr4 = 0xaa;

	_dsync();
}

/**
 * @brief  Restore the pages to a known good state in case of page's status
 *   corruption after a power loss.
 * @param  None.
 * @retval - Flash error code: on write Flash error
 *         - FLASH_COMPLETE: on success
 */
uint32_t EE_Init(void)
{
	uint32_t flash       = 0;
	uint64_t PageStatus0 = 6;
	uint64_t PageStatus1 = 6;
	uint16_t VarIdx = 0;
	uint16_t EepromStatus = 0, ReadStatus = 0;
	int16_t x = -1;
	uint32_t SectorError = 0;

	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Get Page0 status */
	PageStatus0 = (*(uint64_t*)PAGE0_BASE_ADDRESS);
	/* Get Page1 status */
	PageStatus1 = (*(uint64_t*)PAGE1_BASE_ADDRESS);

	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Check for invalid header states and repair if necessary */
	switch (PageStatus0)
	{
	case ERASED:
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
			{
				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE0_BASE_ADDRESS);
				lock_safety_wdtcon();
				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();
			}
		}
		else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
			{
				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE0_BASE_ADDRESS);
				lock_safety_wdtcon();
			}
			/* Mark Page1 as valid */
			/* wait until unbusy */
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			IfxFlash_loadPage2X32(PAGE1_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
			/* write page */
			unlock_safety_wdtcon();
			IfxFlash_writePage(PAGE1_BASE_ADDRESS);
			lock_safety_wdtcon();
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
		}
		else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		}
		break;

	case RECEIVE_DATA:
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
		{
			/* Transfer data from Page1 to Page0 */
			for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++)
			{
				/* Read the last variables' updates */
				ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
				/* In case variable corresponding to the virtual address was found */
				if (ReadStatus != 0x1)
				{
					/* Transfer the variable to the Page0 */
					EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
				}
			}
			/* Mark Page0 as valid */
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			IfxFlash_loadPage2X32(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
			/* write page */
			unlock_safety_wdtcon();
			IfxFlash_writePage(PAGE0_BASE_ADDRESS);
			lock_safety_wdtcon();
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);

			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
			{
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE1_BASE_ADDRESS);
				lock_safety_wdtcon();
			}
		}
		else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
		{
			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
			{
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE1_BASE_ADDRESS);
				lock_safety_wdtcon();
			}
			/* Mark Page0 as valid */
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			IfxFlash_loadPage2X32(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
			/* write page */
			unlock_safety_wdtcon();
			IfxFlash_writePage(PAGE0_BASE_ADDRESS);
			lock_safety_wdtcon();
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
		}
		else /* Invalid state -> format eeprom */
		{
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		}
		break;

	case VALID_PAGE:
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
		{
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		}
		else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
		{
			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
			{
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE1_BASE_ADDRESS);
				lock_safety_wdtcon();
			}
		}
		else /* Page0 valid, Page1 receive */
		{
			/* Transfer data from Page0 to Page1 */
			for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++)
			{
				/* Read the last variables' updates */
				ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
				/* In case variable corresponding to the virtual address was found */
				if (ReadStatus != 0x1)
				{
					/* Transfer the variable to the Page0 */
					EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
				}
			}
			/* Mark Page1 as valid */
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			IfxFlash_loadPage2X32(PAGE1_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
			/* write page */
			unlock_safety_wdtcon();
			IfxFlash_writePage(PAGE1_BASE_ADDRESS);
			lock_safety_wdtcon();
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);

			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
			{
				unlock_safety_wdtcon();
				IfxFlash_eraseSector(PAGE0_BASE_ADDRESS);
				lock_safety_wdtcon();
			}
		}
		break;

	default:  /* Any other state -> format eeprom */
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		/* Erase both Page0 and Page1 and set Page0 as valid page */
		EE_Format();
		break;
	}

	return 0;
}

/**
 * @brief  Verify if specified page is fully erased.
 * @param  Address: page address
 *   This parameter can be one of the following values:
 *     @arg PAGE0_BASE_ADDRESS: Page0 base address
 *     @arg PAGE1_BASE_ADDRESS: Page1 base address
 * @retval page fully erased status:
 *           - 0: if Page not erased
 *           - 1: if Page erased
 */
uint16_t EE_VerifyPageFullyErased(uint32_t addr_start)
{
	uint32_t ReadStatus = 1;
	uint64_t AddressValue;
	uint32_t Address = addr_start;
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Check each active page address starting from end */
	while (Address <= (addr_start+SECTOR_SIZE))
	{
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(uint64_t*)Address);

		/* Compare the read address with the virtual address */
		if (AddressValue != ERASED)
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();

			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;

			break;
		}
		/* Next address location */
		Address = Address + 8;
	}
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
	return ReadStatus;
}

/**
 * @brief  Returns the last stored variable data, if found, which correspond to
 *   the passed virtual address
 * @param  VirtAddress: Variable virtual address
 * @param  Data: Global variable contains the read variable value
 * @retval Success or error status:
 *           - 0: if variable was found
 *           - 1: if the variable was not found
 *           - NO_VALID_PAGE: if no valid page was found.
 */
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint32_t* Data)
{
	uint16_t ValidPage = PAGE0_ID;
	uint16_t AddressValue = EMU_EE_ADDR_START;
	uint16_t ReadStatus = 1;
	uint32_t Address = EEPROM_START_ADDRESS;
	uint32_t PageStartAddress = EEPROM_START_ADDRESS;

	/* Get active Page for read operation */
	ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

	/* Check if there is no valid page */
	if (ValidPage == NO_VALID_PAGE)
	{
		return  NO_VALID_PAGE;
	}

	/* Get the valid Page start Address */
	PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * SECTOR_SIZE));

	/* Get the valid Page end Address */
	Address = (uint32_t)((EEPROM_START_ADDRESS - 8) + (uint32_t)((1 + ValidPage) * SECTOR_SIZE));

	/* Check each active page address starting from end */
	while (Address > (PageStartAddress + 8))
	{
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(uint32_t*)Address);

		/* Compare the read address with the virtual address */
		if (AddressValue == VirtAddress)
		{
			/* Get content of Address-8 which is variable value */
			*Data = (*(uint32_t*)(Address - 8));

			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;

			break;
		}
		else
		{
			/* Next address location */
			Address = Address - 8;
		}
	}

	/* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
	return ReadStatus;
}

/**
 * @brief  Writes/upadtes variable data in EEPROM.
 * @param  VirtAddress: Variable virtual address
 * @param  Data: 32 bit data to be written
 * @retval Success or error status:
 *           - FLASH_COMPLETE: on success
 *           - PAGE_FULL: if valid page is full
 *           - NO_VALID_PAGE: if no valid page was found
 *           - Flash error code: on write Flash error
 */
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint32_t Data)
{
	uint64_t Status = 0;

	/* Write the variable virtual address and value in the EEPROM */
	Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

	/* In case the EEPROM active page is full */
	if (Status == PAGE_FULL)
	{
		/* Perform Page transfer */
		Status = EE_PageTransfer(VirtAddress, Data);
	}

	/* Return last operation status */
	return Status;
}

/**
 * @brief  Erases PAGE and PAGE1 and writes VALID_PAGE header to PAGE
 * @param  None
 * @retval Status of the last operation (Flash write or erase) done during
 *         EEPROM formating
 */
static uint32_t EE_Format(void)
{
	uint32_t flash = 0;
	uint32_t FlashStatus = 0;
	uint32_t SectorError = 0;
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Erase Page0 */
	if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
	{
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		unlock_safety_wdtcon();
		IfxFlash_eraseSector(PAGE0_BASE_ADDRESS);
		lock_safety_wdtcon();
	}
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();
	/* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
	/* Mark Page0 as valid */
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	IfxFlash_loadPage2X32(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
	/* write page */
	unlock_safety_wdtcon();
	IfxFlash_writePage(PAGE0_BASE_ADDRESS);
	lock_safety_wdtcon();
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Erase Page1 */
	if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
	{
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		unlock_safety_wdtcon();
		IfxFlash_eraseSector(PAGE1_BASE_ADDRESS);
		lock_safety_wdtcon();
	}
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();
	return 0;
}

/**
 * @brief  Find valid Page for write or read operation
 * @param  Operation: operation to achieve on the valid page.
 *   This parameter can be one of the following values:
 *     @arg READ_FROM_VALID_PAGE: read operation from valid page
 *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
 * @retval Valid page number (PAGE or PAGE1) or NO_VALID_PAGE in case
 *   of no valid page was found
 */
static uint16_t EE_FindValidPage(uint8_t Operation)
{
	uint64_t PageStatus0 = 6, PageStatus1 = 6;

	/* Get Page0 actual status */
	PageStatus0 = (*(uint64_t*)PAGE0_BASE_ADDRESS);

	/* Get Page1 actual status */
	PageStatus1 = (*(uint64_t*)PAGE1_BASE_ADDRESS);

	/* Write or read operation */
	switch (Operation)
	{
	case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
		if (PageStatus1 == VALID_PAGE)
		{
			/* Page0 receiving data */
			if (PageStatus0 == RECEIVE_DATA)
			{
				return PAGE0_ID;         /* Page0 valid */
			}
			else
			{
				return PAGE1_ID;         /* Page1 valid */
			}
		}
		else if (PageStatus0 == VALID_PAGE)
		{
			/* Page1 receiving data */
			if (PageStatus1 == RECEIVE_DATA)
			{
				return PAGE1_ID;         /* Page1 valid */
			}
			else
			{
				return PAGE0_ID;         /* Page0 valid */
			}
		}
		else
		{
			return NO_VALID_PAGE;   /* No valid Page */
		}

	case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
		if (PageStatus0 == VALID_PAGE)
		{
			return PAGE0_ID;           /* Page0 valid */
		}
		else if (PageStatus1 == VALID_PAGE)
		{
			return PAGE1_ID;           /* Page1 valid */
		}
		else
		{
			return NO_VALID_PAGE ;  /* No valid Page */
		}

	default:
		return PAGE0_ID;             /* Page0 valid */
	}
}


const uint32_t test_dflash_data[] = {
		0x3C70E0F4, 0xB3126DD5, 0xE9B08592, 0x85EA2E14, 0xEDDFF0F5, 0x31112720, 0x6A66F3E2, 0xE5B1D330, 0xA223D471, 0xFCF89257, 0x55CE4051, 0x473562F7, 0x7E5D9E47, 0xEEC4BD06, 0xB8DD2625, 0x07ED2F5A, 0x735892DF, 0x7E0D206E, 0x1EBD451D, 0xCE031F24, 0xAA6AB31C, 0x2CD673B8, 0x7B194BDF, 0xA46A96A5, 0xFB7EEFB2, 0xC939C96F, 0x1CB9EF37, 0x469B11A8, 0x113E33DC, 0x41AC3E9A, 0x56AEE5F2, 0xBD3AD630
};

void dbg_dump(uint32_t addr, uint32_t u32_n){
	//DFlash test
	printf("DFlash[%08X]:\n", addr);
	for(uint32_t i=0; i<u32_n; i+=4) {
		printf("%08X ", *(uint32_t*)(addr+i));
		flush_stdout();
	}
	printf("\n");
}

void DFlashDemo(uint32_t df_sec_n) {
	uint32_t errors = 0;
	uint32_t flash       = 0;
	uint32_t sector_addr = IfxFlash_dFlashTableEepLog[df_sec_n].start;

	/* erase program flash */
	unlock_safety_wdtcon();
	IfxFlash_eraseSector(sector_addr);
	lock_safety_wdtcon();

	dbg_dump(sector_addr, DFLASH_SECTOR_SIZE/16);

	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);

	/* program the given no of pages */
	for (uint16_t page = 0; page < 16; ++page) {
		uint32_t pageAddr = sector_addr + page * IFXFLASH_DFLASH_PAGE_LENGTH;
		errors = IfxFlash_enterPageMode(pageAddr);

		/* wait until unbusy */
		IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);

		IfxFlash_loadPage2X32(pageAddr, test_dflash_data[page*2], test_dflash_data[1+page*2]);
		/* write page */
		unlock_safety_wdtcon();
		IfxFlash_writePage(pageAddr);
		lock_safety_wdtcon();

		/* wait until unbusy */
		IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	}

	dbg_dump(sector_addr, DFLASH_SECTOR_SIZE/16);

	/* Verify the programmed data */
	for (uint16_t page = 0; page < 16; ++page) {
		uint32_t pageAddr = sector_addr + page * IFXFLASH_DFLASH_PAGE_LENGTH;
		uint32_t *addr     = (uint32_t *)pageAddr;

		if((addr[0]==test_dflash_data[page*2]) && (addr[1]==test_dflash_data[1+page*2])) {
		} else {
			printf("err %08X %08X %08X %08X %08X %08X\n",
					&addr[0], &addr[1],
					addr[0], addr[1],
					test_dflash_data[page*2], test_dflash_data[1+page*2]);
			flush_stdout();
			errors ++;
		}
	}

	if (errors) {
		printf("ERROR: error while D-Flash erase / program\n");
	} else {
		printf("OK: D-Flash checks passed\n");
	}
	flush_stdout();
}

/**
 * @brief  Verify if active page is full and Writes variable in EEPROM.
 * @param  VirtAddress: 16 bit virtual address of the variable
 * @param  Data: 32 bit data to be written as variable value
 * @retval Success or error status:
 *           - FLASH_COMPLETE: on success
 *           - PAGE_FULL: if valid page is full
 *           - NO_VALID_PAGE: if no valid page was found
 *           - Flash error code: on write Flash error
 */
static uint16_t EE_VerifyPageFullWriteVariable(uint32_t VirtAddress, uint32_t Data){
	uint32_t flash = 0;
	uint16_t ValidPage = PAGE0_ID;
	uint32_t Address;
	uint32_t PageEndAddress;

	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Get valid Page for write operation */
	ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Check if there is no valid page */
	if (ValidPage == NO_VALID_PAGE)
	{
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		return  NO_VALID_PAGE;
	}

	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Get the valid Page start Address */
	Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * SECTOR_SIZE));
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Get the valid Page end Address */
	PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 1) + (uint32_t)((ValidPage + 1) * SECTOR_SIZE));
	printf("%s %d\n", __func__, __LINE__);
	flush_stdout();

	/* Check each active page address starting from begining */
	while (Address < PageEndAddress)
	{
		printf("%s %d\n", __func__, __LINE__);
		flush_stdout();
		/* Verify if Address and Address+4 contents are ERASED */
		if ((*(uint64_t*)Address) == ERASED)
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			/* Set variable data */
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			IfxFlash_loadPage2X32(Address, (uint32_t)VirtAddress, (uint32_t)(Data));
			/* write page */
			unlock_safety_wdtcon();
			IfxFlash_writePage(Address);
			lock_safety_wdtcon();
			/* wait until unbusy */
			IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
			return 0;
		}
		else
		{
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();
			/* Next address location */
			Address = Address + 8;
		}
	}

	/* Return PAGE_FULL in case the valid page is full */
	return PAGE_FULL;
}

/**
 * @brief  Transfers last updated variables data from the full Page to
 *   an empty one.
 * @param  VirtAddress: 16 bit virtual address of the variable
 * @param  Data: 32 bit data to be written as variable value
 * @retval Success or error status:
 *           - FLASH_COMPLETE: on success
 *           - PAGE_FULL: if valid page is full
 *           - NO_VALID_PAGE: if no valid page was found
 *           - Flash error code: on write Flash error
 */
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint32_t Data)
{
	uint32_t flash = 0;
	uint32_t NewPageAddress = EEPROM_START_ADDRESS;
	uint32_t OldPageAddress = 0;
	uint16_t OldPageId=0;
	uint16_t ValidPage = PAGE0_ID, VarIdx = 0;
	uint16_t EepromStatus = 0, ReadStatus = 0;
	uint32_t SectorError = 0;

	/* Get active Page for read operation */
	ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

	if (ValidPage == PAGE1_ID)       /* Page1 valid */
	{
		/* New page address where variable will be moved to */
		NewPageAddress = PAGE0_BASE_ADDRESS;

		/* Old page address  where variable will be moved from */
		OldPageAddress = PAGE1_BASE_ADDRESS;

		/* Old page ID where variable will be taken from */
		OldPageId = PAGE1_ID;
	}
	else if (ValidPage == PAGE0_ID)  /* Page0 valid */
	{
		/* New page address  where variable will be moved to */
		NewPageAddress = PAGE1_BASE_ADDRESS;

		/* Old page address  where variable will be moved from */
		OldPageAddress = PAGE0_BASE_ADDRESS;

		/* Old page ID where variable will be taken from */
		OldPageId = PAGE0_ID;
	}
	else
	{
		return NO_VALID_PAGE;       /* No valid Page */
	}

	/* Set the new Page status to RECEIVE_DATA status */
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	IfxFlash_loadPage2X32(NewPageAddress, (uint32_t)RECEIVE_DATA, (uint32_t)(RECEIVE_DATA>>32));
	/* write page */
	unlock_safety_wdtcon();
	IfxFlash_writePage(NewPageAddress);
	lock_safety_wdtcon();
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);

	/* Write the variable passed as parameter in the new active page */
	EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

	/* Transfer process: transfer variables from old to the new active page */
	for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++)
	{
		/* Read the last variables' updates */
		ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
		/* In case variable corresponding to the virtual address was found */
		if (ReadStatus != 0x1)
		{
			/* Transfer the variable to the Page0 */
			EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
		}
	}

	if(!EE_VerifyPageFullyErased(OldPageAddress))
	{
		unlock_safety_wdtcon();
		IfxFlash_eraseSector(OldPageAddress);
		lock_safety_wdtcon();
	}


	/* Set new Page status to VALID_PAGE status */
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	IfxFlash_loadPage2X32(NewPageAddress, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
	/* write page */
	unlock_safety_wdtcon();
	IfxFlash_writePage(NewPageAddress);
	lock_safety_wdtcon();
	/* wait until unbusy */
	IfxFlash_waitUnbusy(flash, IfxFlash_FlashType_D0);
	/* Return last operation flash status */

	return 0;
}

#define	Error_Handler() \
		printf("%s %d\n", __func__, __LINE__);	\
		flush_stdout();	\
		{\
			uint32_t df_addr = PAGE0_BASE_ADDRESS;\
			printf("DFlash[%08X]:\n", df_addr);\
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {\
				printf("%08X ", *(uint32_t*)(df_addr+i));\
				flush_stdout();\
			}\
			printf("\n");\
		}\
		\
		{\
			uint32_t df_addr = PAGE1_BASE_ADDRESS;\
			printf("DFlash[%08X]:\n", df_addr);\
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {\
				printf("%08X ", *(uint32_t*)(df_addr+i));\
				flush_stdout();\
			}\
			printf("\n");\
		}	\
		while(1);


void ee_emu_test(void) {
	uint32_t VarDataTab[3];
	uint32_t VarDataTmp;

	/* EEPROM Init */
	if( EE_Init() != 0)
	{
		Error_Handler();
	}

	/* --- Store successively many values of the three variables in the EEPROM ---*/
	/* Store values of Variable1 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++)
	{
		/* Sequence 1 */
		if((EE_WriteVariable(0,  VarValue)) != 0)
		{
			Error_Handler();
		}

		if((EE_ReadVariable(0,  &VarDataTab[0])) != 0)
		{
			Error_Handler();
		}

		if (VarValue != VarDataTab[0])
		{
			Error_Handler();
		}

		/* Sequence 2 */
		if(EE_WriteVariable(1, ~VarValue) != 0)
		{
			Error_Handler();
		}

		if(EE_ReadVariable(1,  &VarDataTab[1]) != 0)
		{
			Error_Handler();
		}

		if(((uint16_t)~VarValue) != VarDataTab[1])
		{
			Error_Handler();
		}

		/* Sequence 3 */
		if(EE_WriteVariable(2,  VarValue << 1) != 0)
		{
			Error_Handler();
		}

		if(EE_ReadVariable(2,  &VarDataTab[2]) != 0)
		{
			Error_Handler();
		}

		if ((VarValue << 1) != VarDataTab[2])
		{
			Error_Handler();
		}

		{
			//DFlash test
			uint32_t df_addr = PAGE0_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}

		{
			//DFlash test
			uint32_t df_addr = PAGE1_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}
	}

	/* Store values of Variable2 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++)
	{
		if(EE_WriteVariable(1, VarValue) != 0)
		{
			Error_Handler();
		}
		if(EE_ReadVariable(1, &VarDataTab[1]) != 0)
		{
			Error_Handler();
		}
		if(VarValue != VarDataTab[1])
		{
			Error_Handler();
		}
		{
			//DFlash test
			uint32_t df_addr = PAGE0_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}

		{
			//DFlash test
			uint32_t df_addr = PAGE1_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}
	}

	/* read the last stored variables data*/
	if(EE_ReadVariable(0, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[0])
	{
		Error_Handler();
	}

	if(EE_ReadVariable(1, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[1])
	{
		Error_Handler();
	}

	if(EE_ReadVariable(2, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[2])
	{
		Error_Handler();
	}

	/* Store values of Variable3 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++)
	{
		if(EE_WriteVariable(2, VarValue) != 0)
		{
			Error_Handler();
		}
		if(EE_ReadVariable(2, &VarDataTab[2]) != 0)
		{
			Error_Handler();
		}
		if(VarValue != VarDataTab[2])
		{
			Error_Handler();
		}
		{
			//DFlash test
			uint32_t df_addr = PAGE0_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}

		{
			//DFlash test
			uint32_t df_addr = PAGE1_BASE_ADDRESS;
			printf("DFlash[%08X]:\n", df_addr);
			for(uint32_t i=0; i<DFLASH_SECTOR_SIZE; i+=4) {
				printf("%08X ", *(uint32_t*)(df_addr+i));
				flush_stdout();
			}
			printf("\n");
		}
	}

	/* read the last stored variables data*/
	if(EE_ReadVariable(0, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[0])
	{
		Error_Handler();
	}

	if(EE_ReadVariable(1, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[1])
	{
		Error_Handler();
	}

	if(EE_ReadVariable(2, &VarDataTmp) != 0)
	{
		Error_Handler();
	}
	if (VarDataTmp != VarDataTab[2])
	{
		Error_Handler();
	}
}
