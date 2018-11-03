//*****************************************************************************
// File: test_M_2.S
// Compiler: HIGHTEC
//******************************************************************************
//	int32_t Ifx_MADD(int32_t A, int32_t B, int32_t C)//
//	int32_t Ifx_MADDS(int32_t A, int32_t B, int32_t C)//
//	uint64_t Ifx_MADD_U(uint64_t A, uint32_t C, uint32_t D)//
//	uint64_t Ifx_MADDS_U(uint32_t A, uint32_t B, uint32_t C)//
//
//	float Ifx_MADD_F(float A, float B, float C)//
//
//	uint64_t Ifx_MADD_H(uint64_t A, uint32_t C, uint32_t D)//
//	uint64_t Ifx_MADDS_H(uint64_t A, uint32_t C, uint32_t D)//
//
//	uint32_t Ifx_MADD_Q(uint32_t A, uint32_t B, uint32_t C)//
//	uint32_t Ifx_MADDS_Q(uint32_t A, uint32_t B, uint32_t C)//
//
//	//Packed Multiply-Add Q Format Multi-precision
//	//Packed Multiply-Add Q Format Multi-precision, Saturated
//	uint64_t Ifx_MADDM_H(uint64_t A, uint32_t C, uint32_t D)//
//	uint64_t Ifx_MADDMS_H(uint64_t A, uint32_t C, uint32_t D)//
//
//	//Packed Multiply-Add Q Format with Rounding
//	//Packed Multiply-Add Q Format with Rounding, Saturated
//	uint32_t Ifx_MADDR_H(uint32_t A, uint32_t B, uint32_t C)//
//	uint32_t Ifx_MADDRS_H(uint32_t A, uint32_t B, uint32_t C)//
//
//	//Multiply-Add Q Format with Rounding
//	//Multiply-Add Q Format with Rounding, Saturated
//	uint32_t Ifx_MADDR_Q(uint32_t A, uint32_t B, uint32_t C)//
//	uint32_t Ifx_MADDRS_Q(uint32_t A, uint32_t B, uint32_t C)//
//
//	//Packed Multiply-Add/Subtract Q Format
//	//Packed Multiply-Add/Subtract Q Format Saturated
//	uint64_t Ifx_MADDSU_H(uint64_t A, uint32_t C, uint32_t D)//
//	uint64_t Ifx_MADDSUS_H(uint64_t A, uint32_t C, uint32_t D)//
//
//	//Packed Multiply-Add/Subtract Q Format Multi-precision
//	//Packed Multiply-Add/Subtract Q Format Multi-precision Saturated
//	uint64_t Ifx_MADDSUM_H(uint64_t A, uint32_t C, uint32_t D)//
//	uint64_t Ifx_MADDSUMS_H(uint64_t A, uint32_t C, uint32_t D)//
//
//	//Packed Multiply-Add/Subtract Q Format with Rounding
//	//Packed Multiply-Add/Subtract Q Format with Rounding Saturated
//	uint32_t Ifx_MADDSUR_H(uint32_t A, uint32_t B, uint32_t C)//
//	uint32_t Ifx_MADDSURS_H(uint32_t A, uint32_t B, uint32_t C)//
//	uint32_t Ifx_Rand32(uint32_t seed, uint32_t B);//
//******************************************************************************
    .global Ifx_MADD
    .global Ifx_MADD_F
    .global Ifx_MADD_H
    .global Ifx_MADD_Q
    .global Ifx_MADD_U
    .global Ifx_MADDM_H
    .global Ifx_MADDMS_H
    .global Ifx_MADDR_H
    .global Ifx_MADDR_Q
    .global Ifx_MADDRS_H
    .global Ifx_MADDRS_Q
    .global Ifx_MADDS
    .global Ifx_MADDS_H
    .global Ifx_MADDS_Q
    .global Ifx_MADDS_U
    .global Ifx_MADDSU_H
    .global Ifx_MADDSUM_H
    .global Ifx_MADDSUMS_H
    .global Ifx_MADDSUR_H
    .global Ifx_MADDSURS_H
    .global Ifx_MADDSUS_H
	.global Ifx_Rand32

#define	inputA       %d4
#define	inputB       %d5
#define	inputC       %d6
#define	inputD       %d7
#define	pos       %d6
#define	inputA64       %e4

#define	tmpD1       %d1
#define	tmpD15       %d15

#define	result    	%d2
#define	result64	%e2
#define	inputA64    	%e4

#define	pointerA       %a4
#define	pointerB      %a5
#define	pointerA64       %p4
#define	pointerResult      %a2
#define	pointerResult64      %p2

    .align 8

//	MADD
//	MADD.F
//	MADD.H
//	MADD.Q
//	MADD.U
//	MADDM.H
//	MADDMS.H
//	MADDR.H
//	MADDR.Q
//	MADDRS.H
//	MADDRS.Q
//	MADDS
//	MADDS.H
//	MADDS.Q
//	MADDS.U
//	MADDSU.H
//	MADDSUM.H
//	MADDSUMS.H
//	MADDSUR.H
//	MADDSURS.H
//	MADDSUS.H

//Multiply-Add
//Multiply-Add, Saturated
//Multiply two signed 32-bit integers, add the product to a signed 32-bit or 64-bit integer and put the result into a 32-
//bit or 64-bit register. The value const9 is sign-extended before the multiplication is performed. The MADDS result
//is saturated on overflow.
Ifx_MADD:
	madd result, inputA, inputB, inputC
	ret

Ifx_MADDS:
	madds result, inputA, inputB, inputC
	ret

Ifx_MADD_U:
	madd.u result64, inputA64, inputC, inputD
	ret

Ifx_MADDS_U:
	madds.u result64, inputA64, inputC, inputD
	ret

//Multiply Add Float
Ifx_MADD_F:
	madd.f result, inputA, inputB, inputC
	ret

//Packed Multiply-Add Q Format
//Packed Multiply-Add Q Format, Saturated
Ifx_MADD_H:
	madd.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MADDS_H:
	madds.h result64, inputA64, inputC, %d7LL, 0
	ret

//Multiply-Add Q Format
//Multiply-Add Q Format, Saturated
Ifx_MADD_Q:
	madd.q result, inputA, inputB, inputC, 0
	ret

Ifx_MADDS_Q:
	madds.q result, inputA, inputB, inputC, 0
	ret

//Packed Multiply-Add Q Format Multi-precision
//Packed Multiply-Add Q Format Multi-precision, Saturated
Ifx_MADDM_H:
	maddm.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MADDMS_H:
	maddms.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Add Q Format with Rounding
//Packed Multiply-Add Q Format with Rounding, Saturated
Ifx_MADDR_H:
	maddr.h result, inputA, inputB, %d6LL, 0
	ret

Ifx_MADDRS_H:
	maddrs.h result, inputA, inputB, %d6LL, 0
	ret

//Multiply-Add Q Format with Rounding
//Multiply-Add Q Format with Rounding, Saturated
Ifx_MADDR_Q:
	maddr.q result, inputA, %d5L, %d6L, 0
	ret

Ifx_MADDRS_Q:
	maddrs.q result, inputA, %d5L, %d6L, 0
	ret

//Packed Multiply-Add/Subtract Q Format
//Packed Multiply-Add/Subtract Q Format Saturated
Ifx_MADDSU_H:
	maddsu.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MADDSUS_H:
	maddsus.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Add/Subtract Q Format Multi-precision
//Packed Multiply-Add/Subtract Q Format Multi-precision Saturated
Ifx_MADDSUM_H:
	maddsum.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MADDSUMS_H:
	maddsums.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Add/Subtract Q Format with Rounding
//Packed Multiply-Add/Subtract Q Format with Rounding Saturated
Ifx_MADDSUR_H:
	maddsur.h result, inputA, inputB, %d6LL, 0
	ret

Ifx_MADDSURS_H:
	maddsurs.h result, inputA, inputB, %d6LL, 0
	ret

//	Implements a linear-congruental random number generator
//	y = (a.Xn + c) mod m
//
//	From Knuth: a=16644525, c=32767
Ifx_Rand32:
	mov inputC, 32767
	madd result, inputB, inputA, inputC
	ret
