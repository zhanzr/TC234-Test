//*****************************************************************************
// File: test_ABS.S
// Compiler: HIGHTEC
//******************************************************************************
// int32_t Ifx_Abs(int32_t X)
// uint32_t Ifx_Abs_B(uint32_t X)
// uint32_t Ifx_Abs_H(uint32_t X)

// INPUTS:
//       input signed 32 bit value
// OUTPUT:
//       None.
//
// RETURN:
//       The absolute value of the input value
//
// DESCRIPTION:
//	Absolute Value
//	Absolute Value Packed Byte
//	Absolute Value Packed Half-word
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
    .global Ifx_Abs
    .global Ifx_Abs_B
    .global Ifx_Abs_H

//Code Segment Declaration
//     .section .text.dsplib, "ax"

#define     input       %d4        //input value
#define     result    	%d2        //result value


//Executable Code
    .align 8
Ifx_Abs:
	abs		result, input
    ret

Ifx_Abs_B:
	abs.b		result, input
    ret

Ifx_Abs_H:
	abs.h		result, input
    ret
