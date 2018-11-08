//*****************************************************************************
// File: test_O.S
// Compiler: HIGHTEC
//******************************************************************************
//uint32_t Ifx_OR(uint32_t A, uint32_t B);
//uint32_t Ifx_OR_T(uint32_t A, uint32_t B);
//uint32_t Ifx_ORN(uint32_t A, uint32_t B);
//uint32_t Ifx_ORN_T(uint32_t A, uint32_t B);
//
//uint32_t Ifx_OR_AND_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_ANDN_T(uint32_t A, uint32_t B, uint32_t C);
//
//uint32_t Ifx_OR_NOR_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_OR_T(uint32_t A, uint32_t B, uint32_t C);
//
//uint32_t Ifx_OR_EQ(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_NE(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_GE(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_GE_U(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_LT(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_OR_LT_U(uint32_t A, uint32_t B, uint32_t C);
//******************************************************************************
 	.global Ifx_OR
 	.global Ifx_OR_AND_T
 	.global Ifx_OR_ANDN_T
 	.global Ifx_OR_EQ
 	.global Ifx_OR_GE
 	.global Ifx_OR_GE_U
 	.global Ifx_OR_LT
 	.global Ifx_OR_LT_U
 	.global Ifx_OR_NE
 	.global Ifx_OR_NOR_T
 	.global Ifx_OR_OR_T
 	.global Ifx_OR_T
 	.global Ifx_ORN
 	.global Ifx_ORN_T

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
//	OR
//	OR.AND.T
//	OR.ANDN.T
//	OR.EQ
//	OR.GE
//	OR.GE.U
//	OR.LT
//	OR.LT.U
//	OR.NE
//	OR.NOR.T
//	OR.OR.T
//	OR.T
//	ORN
//	ORN.T

//Bitwise OR
Ifx_OR:
	or	result, inputA, inputB
	ret

//Bit Logical OR
Ifx_OR_T:
	or.t	result, inputA, 0, inputB, 31
	ret

//Bitwise OR-Not
Ifx_ORN:
	orn	result, inputA, inputB
	ret

//Bit Logical OR-Not
Ifx_ORN_T:
	orn.t result, inputA, 0, inputB, 31
	ret

//Compute the logical operation (AND, ANDN, NOR or OR as appropriate) of the value of bit pos1 of data register
//D[a], and bit pos2 of D[b]. Compute the logical OR of that result and bit [0] of D[c]. Put the result back in bit [0] of
//D[c]. All other bits in D[c] are unchanged.

//Accumulating Bit Logical OR-AND
Ifx_OR_AND_T:
	mov result, inputA
	or.and.t result, inputB, 0, inputC, 31
	ret

//Accumulating Bit Logical OR-AND-Not
Ifx_OR_ANDN_T:
	mov result, inputA
	or.andn.t result, inputB, 0, inputC, 31
	ret

//Accumulating Bit Logical OR-NOR
Ifx_OR_OR_T:
	mov result, inputA
	or.or.t result, inputB, 0, inputC, 31
	ret

//Accumulating Bit Logical OR-OR
Ifx_OR_NOR_T:
	mov result, inputA
	or.nor.t result, inputB, 0, inputC, 31
	ret

//Equal Accumulating
Ifx_OR_EQ:
	mov result, inputA
	or.eq result, inputB, inputC
	ret

//Not Equal Accumulating
Ifx_OR_NE:
	mov result, inputA
	or.ne result, inputB, inputC
	ret

//Greater Than or Equal Accumulating
Ifx_OR_GE:
	mov result, inputA
	or.ge result, inputB, inputC
	ret

//Greater Than or Equal Accumulating Unsigned
Ifx_OR_GE_U:
	mov result, inputA
	or.ge.u result, inputB, inputC
	ret

//Less Than Accumulating Unsigned
Ifx_OR_LT:
	mov result, inputA
	or.lt result, inputB, inputC
	ret

//Less Than Accumulating Unsigned
Ifx_OR_LT_U:
	mov result, inputA
	or.lt.u result, inputB, inputC
	ret


