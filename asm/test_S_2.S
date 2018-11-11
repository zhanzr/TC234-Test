//*****************************************************************************
// File: test_S_2.S
// Compiler: HIGHTEC
//******************************************************************************
//void Ifx_ST_A(uint32_t* pA, uint32_t* pB);
//void Ifx_ST_B(uint32_t* pA, uint32_t B);
//void Ifx_ST_H(uint32_t* pA, uint32_t B);
//void Ifx_ST_W(uint32_t* pA, uint32_t B);
//void Ifx_ST_D(uint32_t* pA, uint64_t B64);
//void Ifx_ST_Q(uint32_t* pA, uint32_t B);
//void Ifx_ST_DA(uint32_t* pA, uint64_t* pC);
//void Ifx_ST_T(void);
//void Ifx_STLCX(uint32_t* pA);
//void Ifx_STUCX(uint32_t* pA);
//void Ifx_SVLCX(void);
//int32_t Ifx_SUB(int32_t A, int32_t B);
//uint32_t* Ifx_SUB_A(uint32_t* pA, uint32_t* pB);
//uint32_t Ifx_SUB_B(uint32_t A, uint32_t B);
//float Ifx_SUB_F(float A, float B);
//uint32_t Ifx_SUB_H(uint32_t A, uint32_t B);
//int32_t Ifx_SUBC(int32_t A, int32_t B);
//int32_t Ifx_SUBX(int32_t A, int32_t B);
//int32_t Ifx_SUBS(int32_t A, int32_t B);
//uint32_t Ifx_SUBS_U(uint32_t A, uint32_t B);
//uint32_t Ifx_SUBS_H(uint32_t A, uint32_t B);
//uint32_t Ifx_SUBS_HU(uint32_t A, uint32_t B);
//void Ifx_SWAP_W(uint32_t* pA, uint32_t A);
//void Ifx_SYSCALL(uint32_t tin);
//******************************************************************************
	.global Ifx_ST_A
	.global Ifx_ST_B
	.global Ifx_ST_D
	.global Ifx_ST_DA
	.global Ifx_ST_H
	.global Ifx_ST_Q
	.global Ifx_ST_T
	.global Ifx_ST_W
	.global Ifx_STLCX
	.global Ifx_STUCX
	.global Ifx_SUB
	.global Ifx_SUB_A
	.global Ifx_SUB_B
	.global Ifx_SUB_F
	.global Ifx_SUB_H
	.global Ifx_SUBC
	.global Ifx_SUBS
	.global Ifx_SUBS_H
	.global Ifx_SUBS_HU
	.global Ifx_SUBS_U
	.global Ifx_SUBX
	.global Ifx_SVLCX
	.global Ifx_SWAP_W
	.global Ifx_SYSCALL

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
//	ST.A
//	ST.B
//	ST.D
//	ST.DA
//	ST.H
//	ST.Q
//	ST.T
//	ST.W
//	STLCX
//	STUCX
//	SUB
//	SUB.A
//	SUB.B
//	SUB.F
//	SUB.H
//	SUBC
//	SUBS
//	SUBS.H
//	SUBS.HU
//	SUBS.U
//	SUBX
//	SVLCX
//	SWAP.W
//	SYSCALL

//Store Word from Address Register
Ifx_ST_A:
	st.a [pointerA], pointerB
	ret

//Store Byte
Ifx_ST_B:
	st.b [pointerA], inputA
	ret

//Store Half-word
Ifx_ST_H:
	st.h  [pointerA], inputA
	ret

//Store Word
Ifx_ST_W:
	st.w [pointerA], inputA
	ret

//Store Double-word
Ifx_ST_D:
	st.d [pointerA], inputA64
	ret

//Store Half-word Signed Fraction
Ifx_ST_Q:
	st.q  [pointerA], inputA
	ret

Ifx_ST_DA:
	st.da [pointerA]+0, pointerC
	ret

//Store Bit
//Store the bit value b to the byte at the memory address specified by off18, in the bit position specified by bpos3.
//The other bits of the byte are unchanged. Individual bits can be used as semaphore
Ifx_ST_T:
//	st.t 0x90000000, 31, 1
	ret

//Subtract
Ifx_SUB:
	sub result, inputA, inputB
	ret

//Subtract Address
Ifx_SUB_A:
	sub.a pointerResult, pointerA, pointerB
	ret

//Subtract Packed Byte
Ifx_SUB_B:
	sub.b result, inputA, inputB
	ret

//Subtract Packed Half-word
Ifx_SUB_H:
	sub.h result, inputA, inputB
	ret

//Subtract Signed with Saturation
Ifx_SUBS:
	subs result, inputA, inputB
	ret

//Subtract Unsigned with Saturation
Ifx_SUBS_U:
	subs.u result, inputA, inputB
	ret

//Subtract Packed Half-word Signed with Saturation
Ifx_SUBS_H:
	subs.h result, inputA, inputB
	ret

//Subtract Packed Half-word Unsigned with Saturation
Ifx_SUBS_HU:
	subs.hu result, inputA, inputB
	ret

//Subtract With Carry
Ifx_SUBC:
	subc result, inputA, inputB
	ret

//Subtract Extended
Ifx_SUBX:
	subx result, inputA, inputB
	ret

//Subtract Float
Ifx_SUB_F:
	sub.f result, inputA, inputB
	ret

//Store Lower Context
Ifx_STLCX:
	stlcx [pointerA], 0
	ret

//Store Upper Context
Ifx_STUCX:
	stucx [pointerA], 0
	ret

//Save Lower Context
Ifx_SVLCX:
	svlcx
	ret

//Swap with Data Register
//Swap atomically the contents of data register D[a] and the memory word specified by the addressing mode.
Ifx_SWAP_W:
	swap.w [pointerA], 0, inputA
	ret

//Jump if Equal
Ifx_Jeq:
	jeq inputA, inputB, ret_3
	mov result, 0
	ret
ret_3:
 	mov result, 1
   	ret

Ifx_SYSCALL:
	jeq inputA, 0, sys_call_0
sys_call_1:
	syscall 1
	ret
sys_call_0:
	syscall 0
	ret
