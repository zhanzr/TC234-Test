//*****************************************************************************
// File: test_T_U_X.S
// Compiler: HIGHTEC
//******************************************************************************
	.global Ifx_TRAPSV
	.global Ifx_TRAPV
	.global Ifx_UPDFL
	.global Ifx_UTOF
	.global Ifx_XNOR
	.global Ifx_XNOR_T
	.global Ifx_XOR
	.global Ifx_XOR_EQ
	.global Ifx_XOR_GE
	.global Ifx_XOR_GE_U
	.global Ifx_XOR_LT
	.global Ifx_XOR_LT_U
	.global Ifx_XOR_NE
	.global Ifx_XOR_T

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
#define	pointerC      %a6
#define	pointerA64       %p4
#define	pointerResult      %a2
#define	pointerResult64      %p2

    .align 8
//	TRAPSV
//	TRAPV
//	UPDFL
//	UTOF
//	XNOR
//	XNOR.T
//	XOR
//	XOR.EQ
//	XOR.GE
//	XOR.GE.U
//	XOR.LT
//	XOR.LT.U
//	XOR.NE
//	XOR.T

//Trap on Sticky Overflow
Ifx_TRAPSV:
	TRAPSV
	ret

//Trap on Overflow
Ifx_TRAPV:
	TRAPV
	ret

//Update Flags
Ifx_UPDFL:
	UPDFL result
	ret

//Unsigned to Floating-point
Ifx_UTOF:
	utof result, inputA
	ret

//Bitwise XOR
Ifx_XOR:
	XOR result, inputA, inputB
	ret

//Bitwise XNOR
Ifx_XNOR:
	XNOR result, inputA, inputB
	ret

//Bit Logical XOR
Ifx_XOR_T:
	mov result, inputA
	xor.t	result, inputB, 0, inputC, 0
	ret

//Bit Logical XNOR
Ifx_XNOR_T:
	mov result, inputA
	xnor.t	result, inputB, 0, inputC, 0
	ret

//Equal Accumulating
Ifx_XOR_EQ:
	mov result, inputA
	XOR.EQ result, inputB, inputC
	ret

//Not Equal Accumulating
Ifx_XOR_NE:
	mov result, inputA
	XOR.NE result, inputB, inputC
	ret

//Greater Than or Equal Accumulating
Ifx_XOR_GE:
	mov result, inputA
	XOR.GE result, inputB, inputC
	ret

//Greater Than or Equal Accumulating Unsigned
Ifx_XOR_GE_U:
	mov result, inputA
	XOR.GE.U result, inputB, inputC
	ret

//Less Than or Equal Accumulating
Ifx_XOR_LT:
	mov result, inputA
	XOR.LT result, inputB, inputC
	ret

//Less Than or Equal Accumulating Unsigned
Ifx_XOR_LT_U:
	mov result, inputA
	XOR.LT.U result, inputB, inputC
	ret
