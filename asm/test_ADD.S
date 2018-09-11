//*****************************************************************************
// File: Ifx_Add.S
// Compiler: HIGHTEC
//******************************************************************************
// int32_t Ifx_Add(int32_t A, int32_t B)
// int32_t Ifx_AddC(int32_t A, int32_t B)
// int32_t Ifx_AddI(int32_t A)
// int32_t Ifx_AddI_Hi(int32_t A)
// int32_t Ifx_Addx(int32_t A, int32_t B)
// int32_t Ifx_Addx_I(int32_t A)
//uint32_t Ifx_Add_B(uint32_t A, uint32_t B);
//uint32_t Ifx_Add_H(uint32_t A, uint32_t B);
//float Ifx_Add_F(float A, float B);

// INPUTS:
// OUTPUT:
//       None.
//
// RETURN:
//
// DESCRIPTION:
//	Add
//	Add with Carry
//  Add Immediate
// Add Extended

// ALGORITHM:
// TECHNIQUES:
//
// ASSUMPTIONS:
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4,d5
//******************************************************************************

//External Reference
    .global Ifx_Add
    .global Ifx_AddC
    .global Ifx_AddI
    .global Ifx_AddI_Hi
    .global Ifx_Addx
    .global Ifx_Addx_I
    .global Ifx_Add_B
    .global Ifx_Add_F
    .global Ifx_Add_H

//Code Segment Declaration
 //    .section .text.dsplib, "ax"
#define     inputA       %d4
#define     inputB       %d5
#define     result    	%d2

//Executable Code
    .align 8
Ifx_Add:
	add		result, inputA, inputB
    ret

Ifx_AddC:
	mov	%d9, 0xffffffff
	mov	%d10, 0xffffffff
	addc %d8, %d9, %d10

	addc		result, inputA, inputB
    ret

Ifx_AddI:
	addi		result, inputA, -10
    ret

Ifx_AddI_Hi:
	addih	result, inputA, 4
    ret

Ifx_Addx:
	addx	result, inputA, inputB
    ret

Ifx_Addx_I:
	addx	result, inputA, -10
    ret

Ifx_Add_B:
	add.b		result, inputA, inputB
    ret

Ifx_Add_H:
	add.h		result, inputA, inputB
    ret

Ifx_Add_F:
	add.f		result, inputA, inputB
    ret
