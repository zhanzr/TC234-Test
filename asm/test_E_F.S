//*****************************************************************************
// File: test_E_F.S
// Compiler: HIGHTEC
//******************************************************************************
//	uint32_t Ifx_Eq(uint32_t A, uint32_t B);
//	uint32_t Ifx_Eq_fast(uint32_t A, uint32_t B);
//	uint32_t Ifx_Eq_B(uint32_t A, uint32_t B);
//	uint32_t Ifx_Eq_H(uint32_t A, uint32_t B);
//	uint32_t Ifx_Eq_W(uint32_t A, uint32_t B);
//	uint32_t Ifx_EqAny_B(uint32_t A, uint32_t B);
//	uint32_t Ifx_EqAny_H(uint32_t A, uint32_t B);
//	uint32_t Ifx_Eq_A(void* pointerA, void* pointerB);
//	uint32_t Ifx_Eq_Z_A(void* pointerA);

//	int32_t Ifx_Ftoi(float inputA);
//	int32_t Ifx_Ftoi_Z(float inputA);
//	uint32_t Ifx_Ftou(float inputA);
//	uint32_t Ifx_Ftou_Z(float inputA);

//	uint32_t Ifx_Extr(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_Extr_U(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_Ftoq31(float inputA, uint32_t inputB);
//	uint32_t Ifx_Ftoq31z(float inputA, uint32_t inputB);
// uint32_t Ifx_Fcall(uint32_t A, uint32_t B);
// uint32_t Ifx_Fcall_A(uint32_t A, uint32_t B);
// uint32_t Ifx_Fcall_I(uint32_t(*p)(uint32_t A, uint32_t B),uint32_t A, uint32_t B);
//******************************************************************************

    .global Ifx_Eq
    .global Ifx_Eq_fast
    .global Ifx_Eq_B
    .global Ifx_Eq_H
    .global Ifx_Eq_W
    .global Ifx_EqAny_B
    .global Ifx_EqAny_H
    .global Ifx_Eq_A
    .global Ifx_Eq_Z_A
    .global Ifx_Ftoi
    .global Ifx_Ftoi_Z
    .global Ifx_Ftou
    .global Ifx_Ftou_Z
    .global Ifx_Ftoq31
    .global Ifx_Ftoq31z
    .global Ifx_Extr
    .global Ifx_Extr_U
    .global Ifx_Fcall
    .global Ifx_Fcall_A
    .global Ifx_Fcall_I

#define	inputA       %d4
#define	inputB       %d5
#define	cond       %d6
#define	pos       %d6
#define	inputC       %d6

#define	tmpD1       %d1
#define	tmpD15       %d15

#define	result    	%d2
#define	result64	%e2
#define	inputA64    	%e4

#define	pointerA       %a4
#define	pointerB      %a5

    .align 8

//Equal
Ifx_Eq:
	eq result, inputA, inputB
    ret

//Equal
Ifx_Eq_fast:
	eq result, inputA, inputB
    fret

//Equal to Address
Ifx_Eq_A:
	eq.a result, pointerA, pointerB
    ret

//Equal Zero Address
Ifx_Eq_Z_A:
	eqz.a result, pointerA
    ret

//Equal Packed Byte
Ifx_Eq_B:
	eq.b result, inputA, inputB
    ret

//Equal Packed Half-word
Ifx_Eq_H:
	eq.h result, inputA, inputB
    ret

 //Equal Packed Wword
Ifx_Eq_W:
	eq.w result, inputA, inputB
    ret

//Equal Any Byte
Ifx_EqAny_B:
	eqany.b result, inputA, inputB
    ret

//Equal Any Half-word
Ifx_EqAny_H:
	eqany.h result, inputA, inputB
    ret

//	Extract Bit Field
Ifx_Extr:
	extr result, inputA, inputB, 4
    ret

//Extract Bit Field Unsigned
Ifx_Extr_U:
	extr.u result, inputA, inputB, 4
    ret

Ifx_Fcall:
	fcall Ifx_Eq_fast
	fret

Ifx_Fcall_A:
	fcalla Ifx_Eq_fast
	fret

Ifx_Fcall_I:
	fcalli pointerA
	fret

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
