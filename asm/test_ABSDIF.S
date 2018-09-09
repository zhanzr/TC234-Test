//*****************************************************************************
// File: Ifx_Absdif.asm
// Compiler: HIGHTEC
//******************************************************************************
// int32_t Ifx_Absdif(int32_t A, int32_t B)
// uint32_t Ifx_Absdif_B(uint32_t A, uint32_t B)
//	Absolute Value of Difference Packed Half-word
// INPUTS:
// OUTPUT:
//       None.
//
// RETURN:
//
// DESCRIPTION:
//	Absolute Value of Difference
//	Absolute Value of Difference Packed Byte
//	Absolute Value of Difference Packed Half-word
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
    .global Ifx_Absdif
    .global Ifx_Absdif_B
    .global Ifx_Absdif_H

//Code Segment Declaration
//     .section .text.dsplib, "ax"

#define     inputA       %d4
#define     inputB       %d5
#define     result    	%d2


//Executable Code
    .align 8
Ifx_Absdif:
	absdif		result, inputA, inputB
    ret

Ifx_Absdif_B:
	absdif.b		result, inputA, inputB
    ret

Ifx_Absdif_H:
	absdif.h		result, inputA, inputB
    ret
