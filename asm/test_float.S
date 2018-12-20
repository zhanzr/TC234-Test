//*****************************************************************************
// File: test_float.S
// Compiler: HIGHTEC
//******************************************************************************
	.global Ifx_Add_F
	.global	Ifx_SUB_F
	.global	Ifx_MUL_F
	.global	Ifx_Div_F
	.global	Ifx_MADD_F
	.global	Ifx_MSUB_F
	.global	Ifx_Cmp_F
	.global	Ifx_Q31TOF
	.global	Ifx_QSEED_F
	.global	Ifx_UTOF
	.global	Ifx_Itof
	.global	Ifx_Ftoi
	.global	Ifx_Ftoi_Z
	.global	Ifx_Ftou
	.global	Ifx_Ftou_Z
	.global	Ifx_Ftoq31
	.global	Ifx_Ftoq31z

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
//	ADD.F
//	SUB.F
//	MUL.F
//	DIV.F
//	MADD.F
//	MSUB.F
//
//	CMP.F
//
//	Q31TOF
//	QSEED.F
//	UTOF
//	FTOI
//	FTOIZ
//	FTOQ31
//	FTOQ31Z
//	FTOU
//	FTOUZ
//	ITOF

Ifx_Add_F:
	add.f result, inputA, inputB
    ret

//Subtract Float
Ifx_SUB_F:
	sub.f result, inputA, inputB
	ret

//Multiply Float
Ifx_MUL_F:
	mul.f result, inputA, inputB
	ret

Ifx_Div_F:
	div.f result, inputA, inputB
	ret

//Multiply Add Float
Ifx_MADD_F:
	madd.f result, inputA, inputB, inputC
	ret

//Multiply Subtract Float
Ifx_MSUB_F:
	MSUB.f result, inputA, inputB, inputC
	ret

// Compare Float
Ifx_Cmp_F:
	cmp.f result, inputA, inputB
	ret

Ifx_Q31TOF:
	Q31TOF result, inputA, inputB
	ret

Ifx_QSEED_F:
	QSEED.F result, inputA
	ret

//Unsigned to Floating-point
Ifx_UTOF:
	utof result, inputA
	ret

//Integer to Float
Ifx_Itof:
	itof result, inputA
	ret

//Float to Integer
Ifx_Ftoi:
	ftoi result, inputA
    ret

//Float to Integer, Round towards Zero
Ifx_Ftoi_Z:
	ftoiz result, inputA
    ret

//Float to Unsigned Integer
Ifx_Ftou:
	ftou result, inputA
    ret

//Float to Unsigned Integer, Round towards Zero
Ifx_Ftou_Z:
	ftouz result, inputA
    ret

//Float to Fraction
Ifx_Ftoq31:
	ftoq31 result, inputA, inputB
    ret

//Float to Fraction, Round towards Zero
Ifx_Ftoq31z:
	ftoq31z result, inputA, inputB
    ret
