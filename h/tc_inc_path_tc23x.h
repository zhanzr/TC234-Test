/*====================================================================
* Project:  Board Support Package (BSP)
* Function: macros for handling different AURIX TC23x devices and boards
*
* Copyright HighTec EDV-Systeme GmbH 1982-2016
*====================================================================*/

#ifndef __TC_INC_PATH_H__
#define __TC_INC_PATH_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__TC23XX__)
# if defined(APPKIT_TC234TFT) || defined(APPKIT_TC237TFT) || defined(TRIBOARD_TC233A) || defined(TRIBOARD_TC234A) || defined(TRIBOARD_TC23XA)
#  define TCPATH	tc23xa
# else
#  error "Unknown TC23x board"
# endif /* APPKIT_TC234TFT */
#else
# error "Unsupported AURIX device"
#endif /* __TC23XX__ */

#define TC_STR(s)		# s
#define TC_INCLUDE(f)	TC_STR(f)

/* check for application kits */
#if defined(APPKIT_TC234TFT) || defined(APPKIT_TC23XX)
# define APPKIT_TC2X4	1
#else
# define APPKIT_TC2X4	0
#endif /* APPKIT_TC2X4 */
#if defined(APPKIT_TC237TFT)
# define APPKIT_TC2X7	1
#else
# define APPKIT_TC2X7	0
#endif /* APPKIT_TC2X7 */
#if (APPKIT_TC2X7 > 0) || (APPKIT_TC2X4 > 0)
# define RUN_ON_APPKIT	1
#else
# define RUN_ON_APPKIT	0
#endif /* APPKIT_TC2XX */

/* check for TriBoard-TC2x3 */
#if defined(TRIBOARD_TC233A)
# define TRIBOARD_TC2X3	1
#else
# define TRIBOARD_TC2X3	0
#endif /* TRIBOARD_TC233A */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TC_INC_PATH_H__ */
