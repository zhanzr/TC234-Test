//*****************************************************************************
// File: test_L.S
// Compiler: HIGHTEC
//******************************************************************************
//	uint32_t* Ifx_LD_A(uint32_t* pA);
//	int8_t Ifx_LD_B(int8_t* pA);
//	uint8_t Ifx_LD_BU(uint8_t* pA);
//	uint64_t Ifx_LD_D(uint64_t* pA);
//	uint64_t* Ifx_LD_DA(uint64_t* pA);
//	int16_t Ifx_LD_H(int16_t* pA);
//	uint16_t Ifx_LD_HU(uint16_t* pA);
//	uint32_t Ifx_LD_Q(uint32_t* pA);
//	int32_t Ifx_LD_W(uint32_t* pA);
//	void Ifx_LDLCX(void);
//	void Ifx_LDUCX(void);
//	uint64_t Ifx_LDMST(uint64_t A);
//	uint32_t* Ifx_LEA(void);
//	uint32_t Ifx_LOOP(uint32_t A, uint32_t B);
//	uint32_t Ifx_LOOPU(uint32_t A, uint32_t B, uint32_t C);
//	uint32_t Ifx_LT(uint32_t A, uint32_t B);
//	uint32_t Ifx_LT_A(uint32_t* pA, uint32_t* pB);
//	pack32 Ifx_LT_B(pack32 packA, pack32 packB);
//	pack32 Ifx_LT_BU(pack32 packA, pack32 packB);
//	pack32 Ifx_LT_H(pack32 packA, pack32 packB);
//	pack32 Ifx_LT_HU(pack32 packA, pack32 packB);
//	uint32_t Ifx_LT_U(uint32_t A, uint32_t B);
//	int32_t Ifx_LT_W(int32_t A, int32_t B);
//	uint32_t Ifx_LT_WU(uint32_t A, uint32_t B);
//******************************************************************************
    .global Ifx_LD_A
    .global Ifx_LD_B
    .global Ifx_LD_BU
    .global Ifx_LD_D
    .global Ifx_LD_DA
    .global Ifx_LD_H
    .global Ifx_LD_HU
    .global Ifx_LD_Q
    .global Ifx_LD_W
    .global Ifx_LDLCX
    .global Ifx_LDMST
    .global Ifx_LDUCX
    .global Ifx_LEA
    .global Ifx_LOOP
    .global Ifx_LOOPU
    .global Ifx_LT
    .global Ifx_LT_A
    .global Ifx_LT_B
    .global Ifx_LT_BU
    .global Ifx_LT_H
    .global Ifx_LT_HU
    .global Ifx_LT_U
    .global Ifx_LT_W
    .global Ifx_LT_WU

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

	//	LD.A
	//	LD.B
	//	LD.BU
	//	LD.D
	//	LD.DA
	//	LD.H
	//	LD.HU
	//	LD.Q
	//	LD.W
	//	LDLCX
	//	LDMST
	//	LDUCX
	//	LEA
	//	LOOP
	//	LOOPU
	//	LT
	//	LT.A
	//	LT.B
	//	LT.BU
	//	LT.H
	//	LT.HU
	//	LT.U
	//	LT.W
	//	LT.WU

//Load Word to Address Register
Ifx_LD_A:
	ld.a pointerResult, [pointerA]
    ret

//Load Byte
Ifx_LD_B:
	ld.b result, [pointerA]
    ret

//Load Byte Unsigned
Ifx_LD_BU:
	ld.bu result, [pointerA]
    ret

//Load Double-word
Ifx_LD_D:
	ld.d result64, [pointerA]
   	ret

//Load Double-word to Address Register
Ifx_LD_DA:
	LD.DA pointerResult, [pointerA]
   	ret

//Load Half-word
Ifx_LD_H:
	ld.h result, [pointerA]
    ret

//Load Half-word Unsigned
Ifx_LD_HU:
	ld.hu result, [pointerA]
    ret

//Load Half-word Signed Fraction
Ifx_LD_Q:
	ld.q result, [pointerA]
    ret

//Load Word
Ifx_LD_W:
	ld.w result, [pointerA]
    ret

//Load Lower Context
Ifx_LDLCX:
	LDLCX 0
    ret

//Load Upper Context
Ifx_LDUCX:
	LDUCX 0
    ret

//Load-Modify-Store
Ifx_LDMST:
	LDMST 8, inputA64
	ret

//LEA
//Load Effective Address
Ifx_LEA:
	lea pointerA, Ifx_LEA
	mov.aa pointerResult, pointerA
    ret

//Loop
Ifx_LOOP:
	mov result, inputA
	mov.a pointerA, inputB
label_loop:
	addi result, result, 1
	loop pointerA, label_loop
	ret

//Loop Unconditional
Ifx_LOOPU:
	mov result, inputA
	mov.a pointerA, inputB
label_loopu:
	addi result, result, 1

	addi inputB, inputB, 1
	jeq inputB, inputC, label_exit

	loopu label_loopu
label_exit:
	ret

//Less Than
Ifx_LT:
	lt result, inputA, inputB
    ret

//Less Than Unsigned
Ifx_LT_U:
	lt.u result, inputA, inputB
    ret

//Less Than Address
Ifx_LT_A:
	lt.a result, pointerA, pointerB
    ret

//Less Than Packed Byte
Ifx_LT_B:
	lt.b result, inputA, inputB
    ret

//Less Than Packed Byte Unsigned
Ifx_LT_BU:
	lt.bu result, inputA, inputB
    ret

//Less Than Packed Half-word
Ifx_LT_H:
	lt.h result, inputA, inputB
    ret

//Less Than Packed Half-word Unsigned
Ifx_LT_HU:
	lt.hu result, inputA, inputB
    ret

//Less Than Packed word
Ifx_LT_W:
	lt.w result, inputA, inputB
    ret

//Less Than Packed word Unsigned
Ifx_LT_WU:
	lt.wu result, inputA, inputB
    ret
