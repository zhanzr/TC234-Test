//*****************************************************************************
// File: test_ABSDIFS.S
// Compiler: HIGHTEC
//******************************************************************************
// uint32_t Ifx_AbsdifS(uint32_t A, uint32_t B)
// uint32_t Ifx_AbsdifS_H(uint32_t A, uint32_t B)

// INPUTS:
//       input       signed 16 bit value
// OUTPUT:
//       None.
//
// RETURN:
//
// DESCRIPTION:
//	Absolute Value of Difference  with Saturation
//	Absolute Value of Difference Packed Half-word with Saturation

// ALGORITHM:
//
// TECHNIQUES:
//       None
//
// ASSUMPTIONS:
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4, d5
//******************************************************************************

//External Reference
    .global Ifx_AbsdifS
    .global Ifx_AbsdifS_H

//Code Segment Declaration
 //    .section .text.dsplib, "ax"

#define     inputA       %d4
#define     inputB       %d5
#define     result    	%d2

//Executable Code
    .align 8
Ifx_AbsdifS:
	absdifs		result, inputA, inputB
    ret

Ifx_AbsdifS_H:
	absdifs.h		result, inputA, inputB
    ret
