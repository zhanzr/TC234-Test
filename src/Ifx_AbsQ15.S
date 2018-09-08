//*****************************************************************************
//
// Copyright (c) 2014 Infineon Technologies AG. All rights reserved.
//
//                                 IMPORTANT NOTICE
//
// Infineon Technologies AG (Infineon) is supplying this file for use
// exclusively with Infineon's microcontroller products. This file can be freely
// distributed within development tools that are supporting such microcontroller
// products.
//
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
// OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************
// File: Ifx_AbsQ15.asm
//
// Compiler: HIGHTEC
//
// $Revision: $
//
// $Date:  $
//
//******************************************************************************
// sint16 Ifx_AbsQ15(sint16 X)
//
// INPUTS:
//       input       signed 16 bit value
//
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
//
//******************************************************************************

//------------------- Section for External Reference ---------------------------

    .global Ifx_AbsQ15

//------------------- Section for Code Segment Declaration ---------------------

     .section .text.dsplib, "ax"

#define     input       %d4        //input value
#define     result    	%d2        //result value


//===============Executable Code================================================

    .align 8
Ifx_AbsQ15:

	abs		result,input		// calculate the absolute value of input and put it into result
    ret

//------------------- END OF FILE ----------------------------------------------
