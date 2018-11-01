//*****************************************************************************
// File: test_ABSS.S
// Compiler: HIGHTEC
//******************************************************************************
// int32_t Ifx_AbsS(int32_t X)
// uint32_t Ifx_AbsS_H(uint32_t X)

// INPUTS:
// OUTPUT:
//
// RETURN:
//       The absolute value of the input value
//
// DESCRIPTION:
//	Absolute Value with Saturation
//	Absolute Value Packed Half-word with Saturation
//
// TECHNIQUES:
//       None
//
// ASSUMPTIONS:
//       1)Inputs are 32 bit signed integers
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4.
//******************************************************************************

//External Reference
    .global Ifx_AbsS
    .global Ifx_AbsS_H

//Code Segment Declaration
//     .section .text.dsplib, "ax"

#define     input       %d4        //input value
#define     result    	%d2        //result value

//Executable Code
    .align 8
Ifx_AbsS:
	abss		result, input
    ret

Ifx_AbsS_H:
	abss.h		result, input
    ret
