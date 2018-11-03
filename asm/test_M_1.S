//*****************************************************************************
// File: test_M_1.S
// Compiler: HIGHTEC
//******************************************************************************
//	uint32_t Ifx_MFCR(void);
//	uint32_t Ifx_MTCR(void);
//
//	uint32_t Ifx_MOV(uint32_t A);
//	uint32_t Ifx_MOV_U(void);
//	uint32_t Ifx_MOVH(void);
//
//	uint32_t* Ifx_MOVH_A(void);
//	uint32_t* Ifx_MOV_A(uint32_t A);
//	uint32_t* Ifx_MOV_AA(uint32_t* pA);
//
//	uint32_t Ifx_MOV_D(uint32_t* pA);
//
//	int32_t Ifx_MAX(int32_t A, int32_t B);
//	uint32_t Ifx_MAX_U(uint32_t A, uint32_t B);
//	pack32 Ifx_MAX_B(pack32 A, pack32 B);
//	pack32 Ifx_MAX_BU(pack32 A, pack32 B);
//	pack32 Ifx_MAX_H(pack32 A, pack32 B);
//	pack32 Ifx_MAX_HU(pack32 A, pack32 B);
//
//	int32_t Ifx_MIN(int32_t A, int32_t B);
//	uint32_t Ifx_MIN_U(uint32_t A, uint32_t B);
//	pack32 Ifx_MIN_B(pack32 A, pack32 B);
//	pack32 Ifx_MIN_BU(pack32 A, pack32 B);
//	pack32 Ifx_MIN_H(pack32 A, pack32 B);
//	pack32 Ifx_MIN_HU(pack32 A, pack32 B);
//
//	int32_t Ifx_MUL(int32_t A, int32_t B);
//	int32_t Ifx_MULS(int32_t A, int32_t B);
//	uint64_t Ifx_MUL_U(uint32_t A, uint32_t B);
//	uint32_t Ifx_MULS_U(uint32_t A, uint32_t B);
//
//	int32_t Ifx_MUL_Q(int32_t A, int32_t B);
//	uint64_t Ifx_MUL_H(uint32_t A, uint32_t B);
//	uint64_t Ifx_MULM_H(uint32_t A, uint32_t B);
//	uint64_t Ifx_MULMS_H(uint32_t A, uint32_t B);
//	int32_t Ifx_MULR_H(int32_t A, int32_t B);
//	int32_t Ifx_MULR_Q(int32_t A, int32_t B);
//
//	float Ifx_MUL_F(float fA, float fB);
//******************************************************************************
    .global Ifx_MFCR
    .global Ifx_MTCR
    .global Ifx_MOV
    .global Ifx_MOV_A
    .global Ifx_MOV_AA
    .global Ifx_MOV_D
    .global Ifx_MOV_U
    .global Ifx_MOVH
    .global Ifx_MOVH_A
    .global Ifx_MAX
    .global Ifx_MAX_B
    .global Ifx_MAX_BU
    .global Ifx_MAX_H
    .global Ifx_MAX_HU
    .global Ifx_MAX_U
    .global Ifx_MIN
    .global Ifx_MIN_B
    .global Ifx_MIN_BU
    .global Ifx_MIN_H
    .global Ifx_MIN_HU
    .global Ifx_MIN_U
    .global Ifx_MUL
    .global Ifx_MUL_U
    .global Ifx_MULS
    .global Ifx_MULS_U
    .global Ifx_MUL_H
    .global Ifx_MUL_Q
    .global Ifx_MULM_H
    .global Ifx_MULMS_H
    .global Ifx_MULR_H
    .global Ifx_MULR_Q
    .global Ifx_MUL_F

#define	inputA       %d4
#define	inputB       %d5
#define	inputC       %d6
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

//	MFCR
//	MTCR
//	MOV
//	MOV.A
//	MOV.AA
//	MOV.D
//	MOV.U
//	MOVH
//	MOVH.A
//	MAX
//	MAX.B
//	MAX.BU
//	MAX.H
//	MAX.HU
//	MAX.U
//	MIN
//	MIN.B
//	MIN.BU
//	MIN.H
//	MIN.HU
//	MIN.U
//	MUL
//	MUL.F
//	MUL.H
//	MUL.Q
//	MUL.U
//	MULM.H
//	MULMS.H
//	MULR.H
//	MULR.Q
//	MULS
//	MULS.U

//Move From Core Register
//MFCR can be executed on any privilege level. This instruction may not be used to access GPRs. Attempting to
//access a GPR with this instruction will return an undefined value.
//Example:
//CPU0_CORE_ID
//CPU0_CPU_ID
//CPU0_CUS_ID
//Ifx_MFCR:
//	mfcr result, CPU0_CORE_ID
//   ret

//Move To Core Register
//Description
//Note: This instruction can only be executed in Supervisor mode.
//The MTCR instruction can not be used to access GPRs. Attempting to update a GPR with this instruction will have
//no effect.
//An MTCR instruction should be followed by an ISYNC instruction. This ensures that all instructions following the
//MTCR see the effects of the CSFR update.
//Ifx_MTCR:
//mtcr val, offset
//	mtcr CPU_D2, 1
//    ret

//mov d3, d1
//mov d3, #-30000
//mov e0, d5; e0 = sign_ext(d5)
//mov e0, d6, d3; e0 = d6_d3 or d1=d6 d0=d3
//mov e0, 1234H; e0 = 0000000000001234H
Ifx_MOV:
	mov result, inputA
    ret

//Move Unsigned
Ifx_MOV_U:
	mov.u result, 0x1234
    ret

//Move High
Ifx_MOVH:
	movh result, 0x1234
    ret

//Move High to Address
Ifx_MOVH_A:
	movh.a pointerResult, 0x1234
    ret

//Move Value to Address Register
Ifx_MOV_A:
	mov.a pointerResult, inputA
   	ret

//Move Address to Data Register
Ifx_MOV_D:
	mov.d result, pointerA
   	ret

//Move Address from Address Register
Ifx_MOV_AA:
	mov.aa pointerResult, pointerA
   	ret

//Maximum Value
//Maximum Value Unsigned
Ifx_MAX:
	max result, inputA, inputB
    ret

Ifx_MAX_U:
	max.u result, inputA, inputB
    ret

//Maximum Value Packed Byte
//Maximum Value Packed Byte Unsigned
Ifx_MAX_B:
	max.b result, inputA, inputB
    ret

Ifx_MAX_BU:
	max.bu result, inputA, inputB
    ret

//Maximum Value Packed Half-word
//Maximum Value Packed Half-word Unsigned
Ifx_MAX_H:
	max.h result, inputA, inputB
    ret

Ifx_MAX_HU:
	max.hu result, inputA, inputB
    ret

//Minimum Value
//Minimum Value Unsigned
Ifx_MIN:
	min result, inputA, inputB
    ret

Ifx_MIN_U:
	min.u result, inputA, inputB
    ret

//Minimum Value Packed Byte
//Minimum Value Packed Byte Unsigned
Ifx_MIN_B:
	min.b result, inputA, inputB
    ret

Ifx_MIN_BU:
	min.bu result, inputA, inputB
    ret

//Minimum Value Packed Half-word
//Minimum Value Packed Half-word Unsigned
Ifx_MIN_H:
	min.h result, inputA, inputB
    ret

Ifx_MIN_HU:
	min.hu result, inputA, inputB
    ret

//Multiply
//Multiply, Saturated
//Multiply Unsigned
//Multiply, Saturated, Unsigned
Ifx_MUL:
	mul result, inputA, inputB
	ret

Ifx_MULS:
	muls result, inputA, inputB
	ret

Ifx_MUL_U:
	mul.u result64, inputA, inputB
	ret

Ifx_MULS_U:
	muls.u result, inputA, inputB
	ret

//Packed Multiply Q Format
Ifx_MUL_H:
	mul.h result64, inputA, %d5LL, 0
	ret

//Multiply Q Format
Ifx_MUL_Q:
	mul.q result, inputA, inputB, 0
	ret

//Packed Multiply Q Format-Multi-precision
Ifx_MULM_H:
	mulm.h result64, inputA, %d5LL, 0
	ret

//Packed Multiply Q Format-Multi-precision, SAT version
//Note: Deprecated in TC2!
Ifx_MULMS_H:
	mulms.h result64, inputA, %d5LL, 0
	ret

//Packed Multiply Q Format with Rounding
Ifx_MULR_H:
	MULR.H result, inputA, %d5LL, 0
	ret

//Multiply Q Format with Rounding
Ifx_MULR_Q:
//	MULR.Q result64, inputA, %d5LL, 0
	mulr.q result, %d4L, %d5L, 0
	ret

//Multiply Float
Ifx_MUL_F:
	mul.f result, inputA, inputB
	ret
