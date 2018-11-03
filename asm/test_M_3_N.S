//*****************************************************************************
// File: test_M_3_N.S
// Compiler: HIGHTEC
//******************************************************************************
//	int32_t Ifx_MSUB(int32_t A, int32_t B, int32_t C);
//	int32_t Ifx_MSUBS(int32_t A, int32_t B, int32_t C);
//	uint64_t Ifx_MSUB_U(uint64_t A, uint32_t C, uint32_t D);
//	uint64_t Ifx_MSUBS_U(uint32_t A, uint32_t B, uint32_t C);
//
//	float Ifx_MSUB_F(float A, float B, float C);
//
//	uint64_t Ifx_MSUB_H(uint64_t A, uint32_t C, uint32_t D);
//	uint64_t Ifx_MSUBS_H(uint64_t A, uint32_t C, uint32_t D);
//
//	uint32_t Ifx_MSUB_Q(uint32_t A, uint32_t B, uint32_t C);
//	uint32_t Ifx_MSUBS_Q(uint32_t A, uint32_t B, uint32_t C);
//
//	//Packed Multiply-Subtract Q Format Multi-precision
//	//Packed Multiply-Subtract Q Format Multi-precision, Saturated
//	uint64_t Ifx_MSUBM_H(uint64_t A, uint32_t C, uint32_t D);
//	uint64_t Ifx_MSUBMS_H(uint64_t A, uint32_t C, uint32_t D);
//
//	//Packed Multiply-Subtract Q Format with Rounding
//	//Packed Multiply-Subtract Q Format with Rounding, Saturated
//	uint32_t Ifx_MSUBR_H(uint32_t A, uint32_t B, uint32_t C);
//	uint32_t Ifx_MSUBRS_H(uint32_t A, uint32_t B, uint32_t C);
//
//	//Multiply-Subtract Q Format with Rounding
//	//Multiply-Subtract Q Format with Rounding, Saturated
//	uint32_t Ifx_MSUBR_Q(uint32_t A, uint32_t B, uint32_t C);
//	uint32_t Ifx_MSUBRS_Q(uint32_t A, uint32_t B, uint32_t C);
//
//	//Packed Multiply-Subtract/Add Q Format
//	//Packed Multiply-Subtract/Add Q Format Saturated
//	uint64_t Ifx_MSUBAD_H(uint64_t A, uint32_t C, uint32_t D);
//	uint64_t Ifx_MSUBADS_H(uint64_t A, uint32_t C, uint32_t D);
//
//	//Packed Multiply-Subtract/Add Q Format Multi-precision
//	//Packed Multiply-Subtract/Add Q Format Multi-precision Saturated
//	uint64_t Ifx_MSUBADM_H(uint64_t A, uint32_t C, uint32_t D);
//	uint64_t Ifx_MSUBADMS_H(uint64_t A, uint32_t C, uint32_t D);
//
//	//Packed Multiply-Subtract/Add Q Format with Rounding
//	//Packed Multiply-Subtract/Add Q Format with Rounding Saturated
//	uint32_t Ifx_MSUBADR_H(uint32_t A, uint32_t B, uint32_t C);
//	uint32_t Ifx_MSUBADRS_H(uint32_t A, uint32_t B, uint32_t C);

// uint32_t Ifx_NAND(uint32_t A, uint32_t B);
// uint32_t Ifx_NAND_T(uint32_t A, uint32_t B);
// uint32_t Ifx_NE(uint32_t A, uint32_t B);
// uint32_t Ifx_NE_A(uint32_t* pA, uint32_t* pB);
// uint32_t Ifx_NEZ_A(uint32_t* pA);
// uint32_t Ifx_NOP(void);
// uint32_t Ifx_NOR(uint32_t A, uint32_t B);
// uint32_t Ifx_NOR_T(uint32_t A, uint32_t B);
// uint32_t Ifx_NOT(uint32_t A);
//******************************************************************************
 	.global Ifx_MSUB
	.global Ifx_MSUBS
	.global Ifx_MSUB_U
	.global Ifx_MSUBS_U
	.global Ifx_MSUB_F
	.global Ifx_MSUB_H
	.global Ifx_MSUBS_H
	.global Ifx_MSUB_Q
	.global Ifx_MSUBS_Q
	.global Ifx_MSUBAD_H
	.global Ifx_MSUBADS_H
	.global Ifx_MSUBADM_H
	.global Ifx_MSUBADMS_H
	.global Ifx_MSUBADR_H
	.global Ifx_MSUBADRS_H
	.global Ifx_MSUBM_H
	.global Ifx_MSUBMS_H
	.global Ifx_MSUBR_H
	.global Ifx_MSUBRS_H
	.global Ifx_MSUBR_Q
	.global Ifx_MSUBRS_Q
	.global Ifx_NAND
	.global Ifx_NAND_T
	.global Ifx_NE
	.global Ifx_NE_A
	.global Ifx_NEZ_A
	.global Ifx_NOP
	.global Ifx_NOR
	.global Ifx_NOR_T
	.global Ifx_NOT

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
//	MSUB
//	MSUBS
//	MSUB.U
//	MSUBS.U
//
//	MSUB.F
//
//	MSUB.H
//	MSUBS.H
//	MSUB.Q
//	MSUBS.Q
//	MSUBAD.H
//	MSUBADS.H
//	MSUBADM.H
//	MSUBADMS.H
//	MSUBADR.H
//	MSUBADRS.H
//	MSUBM.H
//	MSUBMS.H
//	MSUBR.H
//	MSUBRS.H
//	MSUBR.Q
//	MSUBRS.Q
//
//	NAND
//	NAND.T
//	NE
//	NE.A
//	NEZ.A
//	NOP
//	NOR
//	NOR.T
//	NOT (16-bit)
//Multiply-Subtract
//Multiply-Subtract, Saturated
//Multiply two signed 32-bit integers, add the product to a signed 32-bit or 64-bit integer and put the result into a 32-
//bit or 64-bit register. The value const9 is sign-extended before the multiplication is performed. The MSUBS result
//is saturated on overflow.
Ifx_MSUB:
	MSUB result, inputA, inputB, inputC
	ret

Ifx_MSUBS:
	MSUBS result, inputA, inputB, inputC
	ret

Ifx_MSUB_U:
	MSUB.U result64, inputA64, inputC, inputD
	ret

Ifx_MSUBS_U:
	MSUBS.U result64, inputA64, inputC, inputD
	ret

//Multiply Subtract Float
Ifx_MSUB_F:
	MSUB.f result, inputA, inputB, inputC
	ret

//Packed Multiply-Subtract Q Format
//Packed Multiply-Subtract Q Format, Saturated
Ifx_MSUB_H:
	MSUB.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MSUBS_H:
	MSUBs.h result64, inputA64, inputC, %d7LL, 0
	ret

//Multiply-Subtract Q Format
//Multiply-Subtract Q Format, Saturated
Ifx_MSUB_Q:
	MSUB.q result, inputA, inputB, inputC, 0
	ret

Ifx_MSUBS_Q:
	MSUBs.q result, inputA, inputB, inputC, 0
	ret

//Packed Multiply-Subtract Q Format Multi-precision
//Packed Multiply-Subtract Q Format Multi-precision, Saturated
Ifx_MSUBM_H:
	MSUBm.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MSUBMS_H:
	MSUBms.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Subtract Q Format with Rounding
//Packed Multiply-Subtract Q Format with Rounding, Saturated
Ifx_MSUBR_H:
	MSUBr.h result, inputA, inputB, %d6LL, 0
	ret

Ifx_MSUBRS_H:
	MSUBrs.h result, inputA, inputB, %d6LL, 0
	ret

//Multiply-Subtract Q Format with Rounding
//Multiply-Subtract Q Format with Rounding, Saturated
Ifx_MSUBR_Q:
	MSUBr.q result, inputA, %d5L, %d6L, 0
	ret

Ifx_MSUBRS_Q:
	MSUBrs.q result, inputA, %d5L, %d6L, 0
	ret

//Packed Multiply-Subtract/Add Q Format
//Packed Multiply-Subtract/Add Q Format Saturated
Ifx_MSUBAD_H:
	MSUBAD.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MSUBADS_H:
	MSUBADs.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Subtract/Add Q Format Multi-precision
//Packed Multiply-Subtract/Add Q Format Multi-precision Saturated
Ifx_MSUBADM_H:
	MSUBADm.h result64, inputA64, inputC, %d7LL, 0
	ret

Ifx_MSUBADMS_H:
	MSUBADms.h result64, inputA64, inputC, %d7LL, 0
	ret

//Packed Multiply-Subtract/Add Q Format with Rounding
//Packed Multiply-Subtract/Add Q Format with Rounding Saturated
Ifx_MSUBADR_H:
	MSUBADr.h result, inputA, inputB, %d6LL, 0
	ret

Ifx_MSUBADRS_H:
	MSUBADrs.h result, inputA, inputB, %d6LL, 0
	ret

//Bitwise NAND
Ifx_NAND:
	nand	result, inputA, inputB
	ret

//Bitwise Logical NAND
Ifx_NAND_T:
	nand.t result, inputA, 0, inputB, 0
	ret

//Not Equal
Ifx_NE:
	ne result, inputA, inputB
	ret

//Not Equal to Address
Ifx_NE_A:
	ne.a result, pointerA, pointerB
	ret

//Not Equal Zero Address
Ifx_NEZ_A:
	nez.a result, pointerA
	ret

//No Operation
//Used to implement efficient low-power, non-operational instructions.
Ifx_NOP:
	nop
	ret

//Bitwise NOR
Ifx_NOR:
	nor	result, inputA, inputB
	ret

//Bitwise Logical NOR
Ifx_NOR_T:
	nor.t	result, inputA, 0,  inputB, 0
	ret

//Bitwise Complement NOT
// (16-bit)
Ifx_NOT:
	not inputA
	mov result, inputA
	ret

