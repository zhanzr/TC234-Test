//*****************************************************************************
// File: test_S_1.S
// Compiler: HIGHTEC
//******************************************************************************
//int32_t Ifx_SAT_B(int32_t A);
//uint32_t Ifx_SAT_BU(uint32_t A);
//int32_t Ifx_SAT_H(int32_t A);
//uint32_t Ifx_SAT_HU(uint32_t A);
//uint32_t Ifx_SEL(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SELN(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH(uint32_t A, int32_t B);
//uint32_t Ifx_SH_H(uint32_t A, int32_t B);
//uint32_t Ifx_SH_AND_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_ANDN_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_EQ(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_GE(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_GE_U(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_LT(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_LT_U(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_NAND_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_NE(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_NOR_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_OR_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_ORN_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_XNOR_T(uint32_t A, uint32_t B, uint32_t C);
//uint32_t Ifx_SH_XOR_T(uint32_t A, uint32_t B, uint32_t C);
//int32_t Ifx_SHA(int32_t A, int32_t B);
//int32_t Ifx_SHAS(int32_t A, int32_t B);
//uint32_t Ifx_SHA_H(uint32_t A, int32_t B);
//******************************************************************************
	.global Ifx_SAT_B
	.global Ifx_SAT_BU
	.global Ifx_SAT_H
	.global Ifx_SAT_HU
	.global Ifx_SEL
	.global Ifx_SELN
	.global Ifx_SH
	.global Ifx_SH_AND_T
	.global Ifx_SH_ANDN_T
	.global Ifx_SH_EQ
	.global Ifx_SH_GE
	.global Ifx_SH_GE_U
	.global Ifx_SH_H
	.global Ifx_SH_LT
	.global Ifx_SH_LT_U
	.global Ifx_SH_NAND_T
	.global Ifx_SH_NE
	.global Ifx_SH_NOR_T
	.global Ifx_SH_OR_T
	.global Ifx_SH_ORN_T
	.global Ifx_SH_XNOR_T
	.global Ifx_SH_XOR_T
	.global Ifx_SHA
	.global Ifx_SHA_H
	.global Ifx_SHAS

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
//	SAT.B
//	SAT.BU
//	SAT.H
//	SAT.HU
//	SEL
//	SELN
//	SH
//	SH.AND.T
//	SH.ANDN.T
//	SH.EQ
//	SH.GE
//	SH.GE.U
//	SH.H
//	SH.LT
//	SH.LT.U
//	SH.NAND.T
//	SH.NE
//	SH.NOR.T
//	SH.OR.T
//	SH.ORN.T
//	SH.XNOR.T
//	SH.XOR.T
//	SHA
//	SHA.H
//	SHAS

//Saturate Byte
Ifx_SAT_B:
	sat.b result, inputA
	ret

//Saturate Byte Unsigned
Ifx_SAT_BU:
	sat.bu result, inputA
	ret

//Saturate Half-word
Ifx_SAT_H:
	sat.h result, inputA
	ret

//Saturate Half-word Unsigned
Ifx_SAT_HU:
	sat.hu result, inputA
	ret

//Select
Ifx_SEL:
	sel result, inputA, inputB, inputC
	ret

//Select-Not
Ifx_SELN:
	seln result, inputA, inputB, inputC
	ret

//Shift
Ifx_SH:
	sh result, inputA, inputB
	ret

//Shift Equal
Ifx_SH_EQ:
	mov result, inputA
	sh.eq result, inputB, inputC
	ret

//Shift Not Equal
Ifx_SH_NE:
	mov result, inputA
	sh.ne result, inputB, inputC
	ret

//Shift Greater Than or Equal
Ifx_SH_GE:
	mov result, inputA
	sh.ge result, inputB, inputC
	ret

//Shift Greater Than or Equal Unsigned
Ifx_SH_GE_U:
	mov result, inputA
	sh.ge.u result, inputB, inputC
	ret

//Shift Packed Half-words
Ifx_SH_H:
	sh.h result, inputA, inputB
	ret

//Shift Less Than or Equal
Ifx_SH_LT:
	mov result, inputA
	sh.lt result, inputB, inputC
	ret

//Shift Less Than or Equal Unsigned
Ifx_SH_LT_U:
	mov result, inputA
	sh.lt.u result, inputB, inputC
	ret

//Accumulating Shift-AND
Ifx_SH_AND_T:
	mov result, inputA
	sh.and.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-AND-Not
Ifx_SH_ANDN_T:
	mov result, inputA
	sh.andn.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-NAND
Ifx_SH_NAND_T:
	mov result, inputA
	sh.nand.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-OR
Ifx_SH_OR_T:
	mov result, inputA
	sh.or.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-OR-Not
Ifx_SH_ORN_T:
	mov result, inputA
	sh.orn.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-NOR
Ifx_SH_NOR_T:
	mov result, inputA
	sh.nor.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-XOR
Ifx_SH_XOR_T:
	mov result, inputA
	sh.xor.t result, inputB, 0, inputC, 0
	ret

//Accumulating Shift-XNOR
Ifx_SH_XNOR_T:
	mov result, inputA
	sh.xnor.t result, inputB, 0, inputC, 0
	ret

//Arithmetic Shift
Ifx_SHA:
	sha result, inputA, inputB
	ret

//Arithmetic Shift with Saturation
Ifx_SHAS:
	shas result, inputA, inputB
	ret

//Arithmetic Shift Packed Half-words
Ifx_SHA_H:
	sha.h result, inputA, inputB
	ret
