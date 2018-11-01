//*****************************************************************************
// File: test_G_I.S
// Compiler: HIGHTEC
//******************************************************************************
//	uint64_t Ifx_Ge(int32_t inputA, int32_t inputB);
//	uint64_t Ifx_Ge_U(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_Ge_A(void* pointerA, void* pointerB);
//	uint64_t Ifx_Imask(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_InsT(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_Ins_nT(uint32_t inputA, uint32_t inputB);
//	uint32_t Ifx_Insert(uint32_t inputA, uint32_t inputB, uint32_t pos);
//	void Ifx_Isync(void);
//	uint64_t Ifx_Ixmax(uint64_t inputA64, int32_t inputC);
//	uint64_t Ifx_Ixmax_U(uint64_t inputA64, uint32_t inputC);
//	uint64_t Ifx_Ixmin(uint64_t inputA64, int32_t inputC);
//	uint64_t Ifx_Ixmin_U(uint64_t inputA64, uint32_t inputC);
//	float Ifx_Itof(int32_t inputA);
//******************************************************************************
    .global Ifx_Ge
    .global Ifx_Ge_U
    .global Ifx_Ge_A
    .global Ifx_Imask
    .global Ifx_InsT
    .global Ifx_Ins_nT
    .global Ifx_Insert
    .global Ifx_Isync
    .global Ifx_Ixmax
    .global Ifx_Ixmax_U
    .global Ifx_Ixmin
    .global Ifx_Ixmin_U
	.global Ifx_Itof

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

    .align 8

//Greater Than or Equal
Ifx_Ge:
	ge result, inputA, inputB
    ret

//Greater Than or Equal Unsigned
Ifx_Ge_U:
	ge.u result, inputA, inputB
    ret

//Greater Than or Equal Address
Ifx_Ge_A:
	ge.a result, pointerA, pointerB
    ret

//Insert Mask
Ifx_Imask:
	imask result64, inputA, inputB, 8
    ret

//Insert Bit
Ifx_InsT:
	ins.t result, inputA, 5, inputB, 7
    ret

//Insert Bit-Not
Ifx_Ins_nT:
	insn.t result, inputA, 5, inputB, 7
	ret

//Insert Bit Field
Ifx_Insert:
	insert result, inputA, inputB, pos, 8
    ret

//Synchronize Instructions
Ifx_Isync:
	isync
    ret

//Find Maximum Index
//	E[c][15:0] = E[d][15:0] + 2;
//	E[c][63:48] = 00H;
//	if (D[b][15:0] >= D[b][31:16]) AND (D[b][15:0] > E[d][47:32]) then {
//		E[c][47:32] = D[b][15:0];
//		E[c][31:16] = E[d][15:0];
//	} else if (D[b][31:16] > D[b][15:0]) AND (D[b][31:16] > E[d][47:32]) then {
//		E[c][47:32] = D[b][31:16];
//		E[c][31:16] = E[d][15:0]+1;
//	} else {
//		E[c][47:32] = E[d][47:32];
//		E[c][31:16] = E[d][31:16];
//	}
Ifx_Ixmax:
	ixmax result64, inputA64, inputC
    ret

//Find Maximum Index (unsigned)
//	E[c][15:0] = E[d][15:0] + 2;
//	E[c][63:48] = 00H;
//	if (D[b][15:0] >= D[b][31:16]) AND (D[b][15:0] > E[d][47:32]) then {
//		E[c][47:32] = D[b][15:0];
//		E[c][31:16] = E[d][15:0];
//	} else if (D[b][31:16] > D[b][15:0]) AND (D[b][31:16] > E[d][47:32]) then {
//		E[c][47:32] = D[b][31:16];
//		E[c][31:16] = E[d][15:0]+1;
//	} else {
//		E[c][47:32] = E[d][47:32];
//		E[c][31:16] = E[d][31:16];
//	}
Ifx_Ixmax_U:
	ixmax.u result64, inputA64, inputC
    ret

//Find Minimum Index
//	E[c][15:0] = E[d][15:0] + 2;
//	E[c][63:48] = 00H;
//	if (D[b][15:0] <= D[b][31:16]) AND (D[b][15:0] < E[d][47:32]) then {
//		E[c][47:32] = D[b][15:0];
//		E[c][31:16] = E[d][15:0];
//	} else if (D[b][31:16] < D[b][15:0]) AND (D[b][31:16] < E[d][47:32]) then {
//		E[c][47:32] = D[b][31:16];
//		E[c][31:16] = E[d][15:0]+1;
//	} else {
//		E[c][47:32] = E[d][47:32];
//		E[c][31:16] = E[d][31:16];
//	}
Ifx_Ixmin:
	ixmin result64, inputA64, inputC
    ret

//Find Minimum Index (unsigned)
//	E[c][15:0] = E[d][15:0] + 2;
//	E[c][63:48] = 00H;
//	if (D[b][15:0] <= D[b][31:16]) AND (D[b][15:0] < E[d][47:32]) then {
//		E[c][47:32] = D[b][15:0];
//		E[c][31:16] = E[d][15:0];
//	} else if (D[b][31:16] < D[b][15:0]) AND (D[b][31:16] < E[d][47:32]) then {
//		E[c][47:32] = D[b][31:16];
//		E[c][31:16] = E[d][15:0]+1;
//	} else {
//		E[c][47:32] = E[d][47:32];
//		E[c][31:16] = E[d][31:16];
//	}
Ifx_Ixmin_U:
	ixmin.u result64, inputA64, inputC
    ret

//Integer to Float
Ifx_Itof:
	itof result, inputA
	ret

