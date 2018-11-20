//*****************************************************************************
// File: test_ADD.S
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

    .global Ifx_CRC32
    .global Ifx_WAIT

    .global Ifx_TestLED

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
//	movh	inputA, 16270
//	addi	inputA, inputA, 5243
//	movh	inputB, 16398
//	addi	inputB, inputB, 5243
	add.f result, inputA, inputB
    ret

Ifx_CRC32:
//	crc32 result, inputA
	ret

Ifx_WAIT:
	wait
	ret

	.code32

P13_IOCR0 = 0xF003B310
P13_OMR = 0xF003B304

Ifx_TestLED:
//Init LED, P13.0->P13.3
	movh.a	%a2,hi:P13_IOCR0
	lea	%a2,[%a2]lo:P13_IOCR0
	movh %d4, hi:0xC0C0C0C0
	addi %d4, %d4, lo:0xC0C0C0C0
	st.w	[%a2], %d4

	movh.a	%a2,hi:P13_OMR
	lea	%a2,[%a2]lo:P13_OMR

	movh %d4, hi:20
	addi %d4, %d4, lo:20
	mov.a %a4, %d4
forever_loop:
	ld.w %d5,[%a2]
	movh %d4, hi:0x00010001
	addi %d4, %d4, lo:0x00010001
	or %d4, %d4, %d5
	st.w	[%a2], %d4
	fcall test_delay

	ld.w %d5,[%a2]
	movh %d4, hi:0x00020002
	addi %d4, %d4, lo:0x00020002
	or %d4, %d4, %d5
	st.w	[%a2], %d4
	fcall test_delay

	ld.w %d5,[%a2]
	movh %d4, hi:0x00040004
	addi %d4, %d4, lo:0x00040004
	or %d4, %d4, %d5
	st.w	[%a2], %d4
	fcall test_delay

	ld.w %d5,[%a2]
	movh %d4, hi:0x00080008
	addi %d4, %d4, lo:0x00080008
	or %d4, %d4, %d5
	st.w	[%a2], %d4
	fcall test_delay

	loop %a4, forever_loop
	ret

test_delay:
	movh %d6, hi:12000000
	addi %d6, %d6, lo:12000000
	mov.a %a3, %d6
label_loop:
	nop
	loop %a3, label_loop
	fret
