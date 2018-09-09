//*****************************************************************************
// File: Ifx_Adds.asm
// Compiler: HIGHTEC
//******************************************************************************
// int32_t Ifx_AddS(int32_t A, int32_t B)
// uint32_t Ifx_AddS_U(uint32_t A, uint32_t B)
// int32_t Ifx_AddS_H(int32_t A, int32_t B)
// uint32_t Ifx_AddS_HU(uint32_t A, uint32_t B)
//
// INPUTS:
// OUTPUT:
//       None.
//
// RETURN:
//       The absolute value of the input value
//
// DESCRIPTION:
// Add Signed with Saturation
// Add Unsigned with Saturation
// Add Signed Packed Half-word with Saturation
// Add Unsigned Packed Half-word with Saturation
//
// ASSUMPTIONS:
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4, d5
//******************************************************************************

//External Reference
   .global Ifx_AddS
   .global Ifx_AddS_U
   .global Ifx_AddS_H
   .global Ifx_AddS_HU

//Code Segment Declaration
//     .section .text.dsplib, "ax"
#define     inputA       %d4
#define     inputB       %d5
#define     result    	%d2

//Executable Code
    .align 8
Ifx_AddS:
	Adds		result, inputA, inputB
    ret

Ifx_AddS_U:
	Adds.u	result, inputA, inputB
    ret

Ifx_AddS_H:
	Adds.h		result, inputA, inputB
    ret

Ifx_AddS_HU:
	Adds.hu		result, inputA, inputB
    ret
