//*****************************************************************************
// File: test_M_3_N.S
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
	.global Ifx_NOT (16-bit)

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
Ifx_MSUB:
	ret

Ifx_MSUBS:
	ret

Ifx_MSUB_U:
	ret

Ifx_MSUBS_U:
	ret

Ifx_MSUB_F:
	ret

Ifx_MSUB_H:
	ret

Ifx_MSUBS_H:
	ret

Ifx_MSUB_Q:
	ret

Ifx_MSUBS_Q:
	ret

Ifx_MSUBAD_H:
	ret

Ifx_MSUBADS_H:
	ret

Ifx_MSUBADM_H:
	ret

Ifx_MSUBADMS_H:
	ret

Ifx_MSUBADR_H:
	ret

Ifx_MSUBADRS_H:
	ret

Ifx_MSUBM_H:
	ret

Ifx_MSUBMS_H:
	ret

Ifx_MSUBR_H:
	ret

Ifx_MSUBRS_H:
	ret

Ifx_MSUBR_Q:
	ret

Ifx_MSUBRS_Q:
	ret

Ifx_NAND :
	ret

Ifx_NAND_T :
	ret

Ifx_NE :
	ret

Ifx_NE_A :
	ret

Ifx_NEZ_A :
	ret

Ifx_NOP :
	ret

Ifx_NOR :
	ret

Ifx_NOR_T :
	ret

// (16-bit)
Ifx_NOT:
	ret

