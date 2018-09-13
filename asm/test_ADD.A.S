//*****************************************************************************
// File: test_ADD.A.S
// Compiler: HIGHTEC
//******************************************************************************
// void* Ifx_AddA(void* pA, void* pB);
// void* Ifx_AddA_4(void* pA);
// void* Ifx_Addih_A(void* pA);
// void* Ifx_Addsc_A(void* pA, uint32_t scalar);
// void* Ifx_Addsc_AT(void* pA, uint32_t scalar);

// INPUTS:
//       input       signed 16 bit value
// OUTPUT:
//       None.
//
// DESCRIPTION:
//	Add Address
//	Add Immediate Address
//	Add Immediate High to Address

//ADDSC.A
//Add Scaled Index to Address
//ADDSC.AT
//Add Bit-Scaled Index to Address
//Description
//For ADDSC.A, left-shift the contents of data register D[a] by the amount specified by n, where n can be 0, 1, 2, or
//3. Add that value to the contents of address register A[b] and put the result in address register A[c].
//For ADDSC.AT, right-shift the contents of D[a] by three (with sign fill). Add that value to the contents of address
//register A[b] and clear the bottom two bits to zero. Put the result in A[c]. The ADDSC.AT instruction generates the
//address of the word containing the bit indexed by D[a], starting from the base address in A[b]
//
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            a2, a4, a5, d4
//******************************************************************************

//External Reference
    .global Ifx_AddA
    .global Ifx_AddA_4
    .global Ifx_Addih_A
    .global Ifx_Addsc_A
    .global Ifx_Addsc_AT
//Code Segment Declaration
//     .section .text.dsplib, "ax"

#define     inputA       %a4
#define     inputB       %a5
#define		scalar		%d4
#define     result    	%a2
//Executable Code
    .align 8
Ifx_AddA:
	add.a		result, inputA, inputB
    ret

Ifx_AddA_4:
	mov.aa result, inputA
	add.a result, 4
    ret

Ifx_Addih_A:
	addih.a    result, inputA, 4
    ret

Ifx_Addsc_A:
	addsc.a		result, inputA, scalar, 2
    ret

Ifx_Addsc_AT:
	addsc.at	result, inputA, scalar
    ret
