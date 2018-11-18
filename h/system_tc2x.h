/*! \file system_tc2x.h
 *  \brief Extended system control API for TC23x definition
 *
 *  \version
 *
 */

#ifndef __SYSTEM_TC2X_H__
#define __SYSTEM_TC2X_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

/* external oscillator clock (20MHz) */
# define EXTCLK		(20*1000000)

typedef struct _PllInitValue_t
{
	uint32_t valOSCCON;
	uint32_t valPLLCON0;
	uint32_t valPLLCON1;	/* first step K dividers */
	uint32_t valCCUCON0;
	uint32_t valCCUCON1;
	uint32_t valCCUCON2;
	uint32_t finalK;		/* final K2DIV value */
} PllInitValue_t;

void system_clk_config_200_100(void);
void system_clk_config_100_50(void);

void SYSTEM_Init(void);

uint32_t system_GetPllClock(void);

uint32_t system_GetIntClock(void);

uint32_t SYSTEM_GetCpuClock(void);

uint32_t SYSTEM_GetSysClock(void);

uint32_t SYSTEM_GetStmClock(void);

uint32_t SYSTEM_GetCanClock(void);

/*! \brief Globally enable interrupts
 */
void SYSTEM_EnableInterrupts(void);

/*! \brief Globally disable interrupts
 */
void SYSTEM_DisableInterrupts(void);

/*! \brief Globally enable access protection
 *
 *  This function is optional. If the architecture doesn't support access
 *  protection this function does nothing.
 */
void SYSTEM_EnableProtection(void);

/*! \brief Globally disable access protection
 *
 *  This function is optional. If the architecture doesn't support access
 *  protection this function does nothing.
 */
void SYSTEM_DisableProtection(void);

/*! \brief Execute software reset
 */
int SYSTEM_Reset(void);

/*! \brief Execute Idle instruction
 */
int SYSTEM_Idle(void);

/*! \brief Execute power down function
 */
int SYSTEM_Sleep(void);

/*! \brief Debug break system
 */
void SYSTEM_DbgBreak(void);

/*! \brief Check if cache is enabled
 */
int SYSTEM_IsCacheEnabled(void);

/*! \brief Enable/disable cache
 */
void SYSTEM_EnaDisCache(int Enable);

/*   0,1,2 ... core WDT
 *   3     ... sec WDT
 */
void SYSTEM_EnableProtectionExt(int Sel);
void SYSTEM_DisableProtectionExt(int Sel);

void SYSTEM_EnableSecProtection(void);
void SYSTEM_DisableSecProtection(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYSTEM_TC2X_H__ */
