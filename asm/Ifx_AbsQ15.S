//*****************************************************************************
// File: Ifx_AbsQ15.asm
// Compiler: HIGHTEC
//******************************************************************************
// sint16 Ifx_AbsQ15(sint16 X)
//
// INPUTS:
//       input       signed 16 bit value
// OUTPUT:
//       None.
//
// RETURN:
//       The absolute value of the input value
//
// DESCRIPTION:
//
//
// ALGORITHM:
//
// TECHNIQUES:
//       None
//
// ASSUMPTIONS:
//       1)Inputs are 16 bit signed integers
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4.
//******************************************************************************

//External Reference
    .global Ifx_AbsQ15

//Code Segment Declaration
     .section .text.dsplib, "ax"

#define     input       %d4        //input value
#define     result    	%d2        //result value


//Executable Code
    .align 8
Ifx_AbsQ15:

	abs		result, input
    ret
