//*****************************************************************************
// File: test_J.S
// Compiler: HIGHTEC
//******************************************************************************
//	uint32_t Ifx_J(uint32_t inputA);
//	uint32_t Ifx_JA(uint32_t inputA);
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

//******************************************************************************
    .global Ifx_J
    .global Ifx_JA
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
#define	pointerResult      %a2

    .align 8
//	J
//	JA
//	JEQ
//	JEQ.A
//	JGE
//	JGE.U
//	JGEZ (16-bit)
//	JGTZ (16-bit)
//	JI
//	JL
//	JLA
//	JLEZ (16-bit)
//	JLI
//	JLT
//	JLT.U
//	JLTZ (16-bit)
//	JNE
//	JNE.A
//	JNED
//	JNEI
//	JNZ (16-bit)
//	JNZ.A
//	JNZ.T
//	JZ (16-bit)
//	JZ.A
//	JZ.T
//Jump Unconditional
Ifx_J:
	addi result, inputA, 1
	j ret_1
	addi result, inputA, 1
ret_1:
    ret

//Jump Unconditional Absolute
Ifx_JA:
	addi result, inputA, 2
	ja ret_2
	addi result, inputA, 2
ret_2:
    ret

//    Jump Indirect
Ifx_JI:
	mov.aa pointerResult, pointerA
	add.a pointerResult, 4
	ji pointerResult
    ret

//Jump if Equal
Ifx_Jeq:
	jeq inputA, inputB, ret_3
	addi result, inputA, 3
ret_3:
    ret

//Jump if Equal Address
Ifx_Jeq_A:
	jeq.a pointerA, pointerB, ret_4
	mov.aa pointerResult, pointerA
	add.a pointerResult, 4
ret_4:
    ret

//   Jump if Not Equal
Ifx_Jne:
	jne inputA, inputB, ret_14
	addi result, inputA, 14
ret_14:
    ret

//    Jump if Not Equal to Zero Bit
Ifx_Jnz_T:
	jnz.t inputA, 1, ret20
	addi result, inputA, 20
ret20:
    ret

//    Jump if Zero Bit
Ifx_Jz_T:
	jz.t inputA, 1, ret23
	addi result, inputA, 23
ret23:
    ret

//Jump if Not Equal Address
Ifx_Jne_A:
	jne.a pointerA, pointerB, ret_15
	mov.aa pointerResult, pointerA
	add.a pointerResult, 4
ret_15:
    ret

  //Jump if Not Equal to Zero Address
Ifx_Jnz_A:
	jnz.a pointerA, ret_19
	addi result, inputA, 19
ret_19:
    ret

  //Jump if Zero Address
Ifx_Jz_A:
	jz.a pointerA, ret_22
	addi result, inputA, 22
ret_22:
    ret

 //Jump if Not Equal and Decrement
Ifx_Jned:
	jned inputA, inputB, ret_16
	addi result, inputA, 16
ret_16:
    ret

  //Jump if Not Equal and Increment
Ifx_Jnei:
	jnei inputA, inputB, ret_17
	addi result, inputA, 17
ret_17:
    ret

//     Jump if Greater Than or Equal
Ifx_Jge:
	jge inputA, inputB, ret_5
	addi result, inputA, 5
ret_5:
    ret

//Jump if Greater Than or Equal Unsigned
Ifx_Jge_U:
	jge.u inputA, inputB, ret_6
	addi result, inputA, 6
ret_6:
    ret

//Jump if Greater Than or Equal to Zero (16-bit)
Ifx_Jgez:
	jgez inputA, ret_7
	addi result, inputA, 7
ret_7:
    ret

 //Jump if Greater Than Zero (16-bit)
Ifx_Jgtz:
	jgtz inputA, ret_8
	addi result, inputA, 8
ret_8:
    ret

 //Jump if Less Than Zero (16-bit)
Ifx_Jltz:
	jgtz inputA, ret_13
	addi result, inputA, 13
ret_13:
    ret

  //Jump if Not Equal to Zero (16-bit)
Ifx_Jnz:
	jnz inputA, ret_18
	addi result, inputA, 18
ret_18:
    ret

 //Jump if Zero (16-bit)
Ifx_Jz:
	jnz inputA, ret_21
	addi result, inputA, 21
ret_21:
    ret

//Jump and Link
Ifx_JL:
	addi result, inputA, 9
	jl ret_9
	addi result, inputA, 9
ret_9:
    ret

 //Jump and Link Absolute
Ifx_JLA:
	addi result, inputA, 10
	jla ret_10
	addi result, inputA, 10
ret_10:
    ret

 //Jump if Less Than or Equal to Zero (16-bit)
Ifx_Jlez:
	jlez inputA, ret_11
	addi result, inputA, 11
ret_11:
    ret

// Jump and Link Indirect
Ifx_JLI:
	mov.aa pointerResult, pointerA
	add.a pointerResult, 4
	jli pointerResult
    ret

//     Jump if Less Than or Equal
Ifx_Jlt:
	jge inputA, inputB, ret_12
	addi result, inputA, 12
ret_12:
    ret

//Jump if Less Than or Equal Unsigned
Ifx_Jlt_U:
	jge.u inputA, inputB, ret_24
	addi result, inputA, 24
ret_24:
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

