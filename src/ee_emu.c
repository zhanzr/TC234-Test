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

void IfxFlash_eraseSector(uint32_t sectorAddr) {
	volatile uint32_t *addr1 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa50);
	volatile uint32_t *addr2 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa58);
	volatile uint32_t *addr3 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);
	volatile uint32_t *addr4 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);

	*addr1 = sectorAddr;
	*addr2 = 1;
	*addr3 = 0x80;
	*addr4 = 0x50;

	_dsync();
}

void IfxFlash_loadPage2X32(uint32_t pageAddr, uint32_t wordL, uint32_t wordU) {
	volatile uint32_t *addr1 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0x55f0);

	*addr1 = wordL;
	addr1++;
	*addr1 = wordU;

	_dsync();
}

uint8_t IfxFlash_waitUnbusy(uint32_t flash, IfxFlash_FlashType flashType) {
	if (flash == 0) {
		while (FLASH0_FSR.U & (1 << flashType)){
			;
		}
	} else {
		return 1; // invalid flash selected
	}
	_dsync();
	return 0;     // finished
}

uint8_t IfxFlash_enterPageMode(uint32_t pageAddr) {
	volatile uint32_t *addr1 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0x5554);

	if ((pageAddr & 0xff000000) == 0xa0000000) {
		*addr1 = 0x50;
		return 0;
	} else if ((pageAddr & 0xff000000) == 0xaf000000) {
		*addr1 = 0x5D;
		return 0;
	}

	_dsync();
	return 1; // invalid flash address
}


void IfxFlash_writePage(uint32_t pageAddr) {
	volatile uint32_t *addr1 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa50);
	volatile uint32_t *addr2 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaa58);
	volatile uint32_t *addr3 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);
	volatile uint32_t *addr4 = (volatile uint32_t *)(IFXFLASH_CMD_BASE_ADDRESS | 0xaaa8);

	*addr1 = pageAddr;
	*addr2 = 0x00;
	*addr3 = 0xa0;
	*addr4 = 0xaa;

	_dsync();
}

void dbg_dump(uint32_t addr, uint32_t u32_n) {
	//DFlash test
	printf("DFlash[%08X]:\n", addr);
	for(uint32_t i=0; i<u32_n; i+=4) {
		printf("%08X ", *(uint32_t*)(addr+i));
	}
	printf("\n");
	flush_stdout();
}

void erase_sector(uint32_t s_addr) {
	/* erase program flash */
	unlock_safety_wdtcon();
	IfxFlash_eraseSector(s_addr);
	lock_safety_wdtcon();
}

inline void program_page(uint32_t page_addr, uint32_t u32_0, uint32_t u32_1) {
	printf("P[%08X]:%08X %08X\n", page_addr, u32_0, u32_1);

	IfxFlash_enterPageMode(page_addr);

	/* wait until unbusy */
	IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);

	IfxFlash_loadPage2X32(page_addr, u32_0, u32_1);
	/* write page */
	unlock_safety_wdtcon();
	IfxFlash_writePage(page_addr);
	lock_safety_wdtcon();

	/* wait until unbusy */
	IfxFlash_waitUnbusy(0, IfxFlash_FlashType_D0);
}

void DFlashDemo(uint8_t dflash_sec_n) {
	uint32_t sector_addr = IFXFLASH_DFLASH_START + dflash_sec_n*DFLASH_SECTOR_SIZE;

	erase_sector(sector_addr);

	dbg_dump(sector_addr, DFLASH_SECTOR_SIZE/32);

	/* program the given no of pages */
	for (uint16_t page = 0; page < 16; ++page) {
		uint32_t pageAddr = sector_addr + page * IFXFLASH_DFLASH_PAGE_LENGTH;
		uint32_t u32_0 = rand();
		uint32_t u32_1 = rand();
		program_page(pageAddr, u32_0, u32_1);
	}

	dbg_dump(sector_addr, DFLASH_SECTOR_SIZE/32);
}

/**
 * @brief  Restore the pages to a known good state in case of page's status
 *   corruption after a power loss.
 * @param  None.
 * @retval - Flash error code: on write Flash error
 *         - FLASH_COMPLETE: on success
 */
uint32_t EE_Init(void) {
	uint64_t PageStatus0;
	uint64_t PageStatus1;
	uint16_t VarIdx = 0;
	uint32_t ReadStatus = 1;
	uint16_t EepromStatus = 0;

	/* Get Page0 status */
	PageStatus0 = (*(uint64_t*)PAGE0_BASE_ADDRESS);
	/* Get Page1 status */
	PageStatus1 = (*(uint64_t*)PAGE1_BASE_ADDRESS);

	printf("%s %d, p0_st:%016llX, p1_st:%016llX\n", __func__, __LINE__, PageStatus0, PageStatus1);
	flush_stdout();

	/* Check for invalid header states and repair if necessary */
	switch (PageStatus0)
	{
	case ERASED:
		printf("%s %d p0 erased \n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) {
		  /* Page0 erased, Page1 valid */
			printf("%s %d Page0 erased, Page1 valid\n", __func__, __LINE__);
			flush_stdout();
			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS)) {
				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();

				erase_sector(PAGE0_BASE_ADDRESS);

				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();
			}
		} else if (PageStatus1 == RECEIVE_DATA) {
			printf("%s %d  /* Page0 erased, Page1 receive */\n", __func__, __LINE__);
			flush_stdout();
			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
			{
				printf("%s %d\n", __func__, __LINE__);
				flush_stdout();

				erase_sector(PAGE0_BASE_ADDRESS);
			}
			/* Mark Page1 as valid */
			/* wait until unbusy */
			printf("%s %d\n", __func__, __LINE__);
			flush_stdout();

			program_page(PAGE1_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
		} else {
			printf("%s %d both page erased, need format\n", __func__, __LINE__);
			flush_stdout();
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		}
		break;

	case RECEIVE_DATA:
		printf("%s %d p0 rcv\n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) {
			/* Page0 receive, Page1 valid */
			/* Transfer data from Page1 to Page0 */
			for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++) {
				/* Read the last variables' updates */
				ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
				/* In case variable corresponding to the virtual address was found */
				if (ReadStatus != 0x1) {
					/* Transfer the variable to the Page0 */
					EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
				}
			}
			/* Mark Page0 as valid */
			program_page(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));

			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS)) {
				erase_sector(PAGE1_BASE_ADDRESS);
			}
		}
		else if (PageStatus1 == ERASED)	{
			/* Page0 receive, Page1 erased */
			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS)) {
				erase_sector(PAGE1_BASE_ADDRESS);
			}
			/* Mark Page0 as valid */
			program_page(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));
		} else {
			/* Invalid state -> format eeprom */
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		}
		break;

	case VALID_PAGE:
		printf("%s %d p0 valid\n", __func__, __LINE__);
		flush_stdout();
		if (PageStatus1 == VALID_PAGE) {
			/* Invalid state -> format eeprom */
			/* Erase both Page0 and Page1 and set Page0 as valid page */
			EE_Format();
		} else if (PageStatus1 == ERASED) {
			 /* Page0 valid, Page1 erased */
			/* Erase Page1 */
			if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS)) {
				erase_sector(PAGE1_BASE_ADDRESS);
			}
		} else {
			/* Page0 valid, Page1 receive */
			/* Transfer data from Page0 to Page1 */
			for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++) {
				/* Read the last variables' updates */
				ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
				/* In case variable corresponding to the virtual address was found */
				if (ReadStatus != 0x1) {
					/* Transfer the variable to the Page1 */
					EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
				}
			}
			/* Mark Page1 as valid */
			program_page(PAGE1_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));

			/* Erase Page0 */
			if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS)) {
				erase_sector(PAGE0_BASE_ADDRESS);
			}
		}
		break;

	default:  /* Any other state -> format eeprom */
		printf("%s %d p0 any other st, go format\n", __func__, __LINE__);
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
uint16_t EE_VerifyPageFullyErased(uint32_t addr_start) {
	uint32_t ReadStatus = 1;
	uint64_t AddressValue;
	uint32_t Address = addr_start;

	/* Check each active page address starting from end */
	while (Address <= (addr_start+SECTOR_SIZE)) {
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(uint64_t*)Address);

		/* Compare the read address with the virtual address */
		if (AddressValue != ERASED) {
			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;
			break;
		}
		/* Next address location */
		Address = Address + IFXFLASH_DFLASH_PAGE_LENGTH;
	}
	printf("%s %d %08X :%s\n", __func__, __LINE__, addr_start, (0==ReadStatus)?"Dirty":"Erased");
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
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint32_t* pData) {
	uint16_t ValidPage = PAGE0_ID;
	uint16_t AddressValue;
	uint16_t ReadStatus = 1;
	uint32_t Address;
	uint32_t PageStartAddress;

	printf("%s %d va:%08X\n", __func__, __LINE__, VirtAddress);
	flush_stdout();

	/* Get active Page for read operation */
	ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

	/* Check if there is no valid page */
	if (ValidPage == NO_VALID_PAGE) {
		return  NO_VALID_PAGE;
	}

	/* Get the valid Page start Address */
	PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * SECTOR_SIZE));

	/* Get the valid Page end Address */
	Address = (uint32_t)((EEPROM_START_ADDRESS - IFXFLASH_DFLASH_PAGE_LENGTH) +
			(uint32_t)((1 + ValidPage) * SECTOR_SIZE));

	printf("%s %d search:%08X-%08X\n", __func__, __LINE__, PageStartAddress, Address);
	flush_stdout();
	/* Check each active page address starting from end */
	while (Address >= (PageStartAddress + IFXFLASH_DFLASH_PAGE_LENGTH)) {
		/* Get the current location content to be compared with virtual address */
		AddressValue = (*(uint32_t*)Address);

//		printf("...:[%08X]=%08X\t",  Address, AddressValue);
//		flush_stdout();
		/* Compare the read address with the virtual address */
		if (AddressValue == VirtAddress) {
			/* Get content*/
			*pData = (*(uint32_t*)(Address + 4));

			printf("%s %d got va:%08X ra:%08X dat:%08X\n", __func__, __LINE__, VirtAddress, Address, *pData);
			flush_stdout();

			/* In case variable value is read, reset ReadStatus flag */
			ReadStatus = 0;

			break;
		} else {
			/* Next address location */
			Address = Address - IFXFLASH_DFLASH_PAGE_LENGTH;
		}
	}

	printf("%s %d ret %u\n", __func__, __LINE__, ReadStatus);
	flush_stdout();
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
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint32_t Data) {
	uint64_t Status = 0;

	printf("%s %d va:%08X dat:%08X\n", __func__, __LINE__, VirtAddress, Data);
	flush_stdout();

	/* Write the variable virtual address and value in the EEPROM */
	Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

	/* In case the EEPROM active page is full */
	if (Status == PAGE_FULL) {
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
static uint32_t EE_Format(void) {
	/* Erase Page0 */
	if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS)) {
		printf("%s %d, need erase %08X\n", __func__, __LINE__, PAGE0_BASE_ADDRESS);
		flush_stdout();

		erase_sector(PAGE0_BASE_ADDRESS);
	}
	printf("%s %d make valid for %08X\n", __func__, __LINE__, PAGE0_BASE_ADDRESS);
	flush_stdout();
	/* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
	/* Mark Page0 as valid */
	program_page(PAGE0_BASE_ADDRESS, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));

	dbg_dump(PAGE0_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);

	/* Erase Page1 */
	if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS)) {
		printf("%s %d, need erase %08X\n", __func__, __LINE__, PAGE1_BASE_ADDRESS);
		flush_stdout();

		erase_sector(PAGE1_BASE_ADDRESS);
	}

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
static uint16_t EE_VerifyPageFullWriteVariable(uint32_t VirtAddress, uint32_t Data) {
	uint32_t flash = 0;
	uint16_t ValidPage = PAGE0_ID;
	uint32_t Address;
	uint32_t PageEndAddress;

	/* Get valid Page for write operation */
	ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

	printf("%s %d va:%08X:dat:%08X,vp:%02X\n", __func__, __LINE__, VirtAddress, Data, ValidPage);
	flush_stdout();

	/* Check if there is no valid page */
	if (ValidPage == NO_VALID_PAGE) {
		return  NO_VALID_PAGE;
	}

	/* Get the valid Page start Address */
	Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * SECTOR_SIZE));

	/* Get the valid Page end Address */
	PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 1) + (uint32_t)((ValidPage + 1) * SECTOR_SIZE));
	printf("%s %d start:%08X, end:%08X\n", __func__, __LINE__, Address, PageEndAddress);
	flush_stdout();

	/* Check each active page address starting from begining */
	while (Address < PageEndAddress) {
		/* Verify if Address and Address+4 contents are ERASED */
		if ((*(uint64_t*)Address) == ERASED) {
			printf("%s %d found blank slot %08X\n", __func__, __LINE__, Address);
			flush_stdout();
			/* Set variable data */
			program_page(Address, (uint32_t)VirtAddress, (uint32_t)(Data));

			return 0;
		} else {
			/* Next address location */
			Address = Address + IFXFLASH_DFLASH_PAGE_LENGTH;
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
	program_page(NewPageAddress, (uint32_t)RECEIVE_DATA, (uint32_t)(RECEIVE_DATA>>32));

	/* Write the variable passed as parameter in the new active page */
	EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

	/* Transfer process: transfer variables from old to the new active page */
	for (VarIdx = EMU_EE_ADDR_START; VarIdx < EMU_EE_ADDR_END; VarIdx++)
	{
		/* Read the last variables' updates */
		ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
		/* In case variable corresponding to the virtual address was found */
		if (ReadStatus != 0x1) {
			/* Transfer the variable to the new page */
			EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
		}
	}

	if(!EE_VerifyPageFullyErased(OldPageAddress))
	{
		erase_sector(OldPageAddress);
	}

	/* Set new Page status to VALID_PAGE status */
	program_page(NewPageAddress, (uint32_t)VALID_PAGE, (uint32_t)(VALID_PAGE>>32));

	return 0;
}

#define	Error_Handler() \
		printf("%s %d\n", __func__, __LINE__);	\
		flush_stdout();	\
		dbg_dump(PAGE0_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);\
		dbg_dump(PAGE1_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);\
		erase_sector(PAGE0_BASE_ADDRESS);\
		erase_sector(PAGE1_BASE_ADDRESS);\
		while(1);


void ee_emu_test(void) {
//	DFlashDemo(0);
//	DFlashDemo(1);
//	DFlashDemo(2);
//
//	return;

	uint32_t VarDataTab[] = {0x12345678, 0x55aacc33, 0x816495ac};
	uint32_t VarDataTmp;

	/* EEPROM Init */
	if( EE_Init() != 0) {
		Error_Handler();
	}

	/* --- Store successively many values of the three variables in the EEPROM ---*/
	/* Store values of Variable1 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++)
	{
		/* Sequence 1 */
		if((EE_WriteVariable(EMU_EE_ADDR_START,  VarValue)) != 0) {
			Error_Handler();
		}

		if((EE_ReadVariable(EMU_EE_ADDR_START,  &VarDataTab[0])) != 0) {
			Error_Handler();
		}

		if (VarValue != VarDataTab[0])
		{
			Error_Handler();
		}

		/* Sequence 2 */
		if(EE_WriteVariable(EMU_EE_ADDR_START+1, ~VarValue) != 0) {
			Error_Handler();
		}

		if(EE_ReadVariable(EMU_EE_ADDR_START+1,  &VarDataTab[1]) != 0) {
			Error_Handler();
		}

		/* Sequence 3 */
		if(EE_WriteVariable(EMU_EE_ADDR_START+2,  VarValue << 1) != 0)
		{
			Error_Handler();
		}

		if(EE_ReadVariable(EMU_EE_ADDR_START+2,  &VarDataTab[2]) != 0)
		{
			Error_Handler();
		}

		dbg_dump(PAGE0_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
		dbg_dump(PAGE1_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
	}

	/* Store values of Variable2 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++) {
		if(EE_WriteVariable(EMU_EE_ADDR_START + 1, VarValue) != 0) {
			Error_Handler();
		}

		if(EE_ReadVariable(EMU_EE_ADDR_START + 1, &VarDataTab[1]) != 0) {
			Error_Handler();
		}

		if(VarValue != VarDataTab[1]) {
			Error_Handler();
		}
		dbg_dump(PAGE0_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
		dbg_dump(PAGE1_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
	}

	/* read the last stored variables data*/
	if(EE_ReadVariable(EMU_EE_ADDR_START+0, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[0]) {
		Error_Handler();
	}

	if(EE_ReadVariable(EMU_EE_ADDR_START + 1, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[1]) {
		Error_Handler();
	}

	if(EE_ReadVariable(EMU_EE_ADDR_START + 2, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[2]) {
		Error_Handler();
	}

	/* Store values of Variable3 in EEPROM */
	for (uint32_t VarValue = 1; VarValue <= 2; VarValue++) {
		if(EE_WriteVariable(EMU_EE_ADDR_START + 2, VarValue) != 0) {
			Error_Handler();
		}

		if(EE_ReadVariable(EMU_EE_ADDR_START + 2, &VarDataTab[2]) != 0) {
			Error_Handler();
		}

		if(VarValue != VarDataTab[2]) {
			Error_Handler();
		}

		dbg_dump(PAGE0_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
		dbg_dump(PAGE1_BASE_ADDRESS, DFLASH_SECTOR_SIZE/32);
	}

	/* read the last stored variables data*/
	if(EE_ReadVariable(EMU_EE_ADDR_START + 0, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[0]) {
		Error_Handler();
	}

	if(EE_ReadVariable(EMU_EE_ADDR_START + 1, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[1]) {
		Error_Handler();
	}

	if(EE_ReadVariable(EMU_EE_ADDR_START + 2, &VarDataTmp) != 0) {
		Error_Handler();
	}

	if (VarDataTmp != VarDataTab[2]) {
		Error_Handler();
	}
}
