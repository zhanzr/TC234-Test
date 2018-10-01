//*****************************************************************************
// File: test_J.S
// Compiler: HIGHTEC
//******************************************************************************
//uint32_t Ifx_J(uint32_t inputA);
//uint32_t Ifx_JA(uint32_t inputA);
//uint32_t Ifx_JI(uint32_t (*pFunc)(uint32_t inputA));
//uint32_t Ifx_Jeq(uint32_t A, uint32_t B);
//void* Ifx_Jeq_A(void* A, void* B);
//uint32_t Ifx_Jne(uint32_t A, uint32_t B);
//uint32_t Ifx_Jnz_T(uint32_t A);
//uint32_t Ifx_Jz_T(uint32_t A);
//void* Ifx_Jne_A(void* A, void* B);
//void* Ifx_Jnz_A(void* A, void* B);
//void* Ifx_Jz_A(void* A, void* B);
//
//uint32_t Ifx_Jned(uint32_t A, uint32_t B);
//uint32_t Ifx_Jnei(uint32_t A, uint32_t B);
//uint32_t Ifx_Jge(uint32_t A, uint32_t B);
//uint32_t Ifx_Jge_U(uint32_t A, uint32_t B);
//uint32_t Ifx_Jgez(uint32_t A);
//uint32_t Ifx_Jgtz(uint32_t A);
//uint32_t Ifx_Jltz(uint32_t A);
//uint32_t Ifx_Jnz(uint32_t A);
//uint32_t Ifx_Jz(uint32_t A);
//uint32_t Ifx_JL(uint32_t A);
//uint32_t Ifx_JLA(uint32_t A, uint32_t B);
//uint32_t Ifx_Jlez(uint32_t A);
//uint32_t Ifx_JLI(uint32_t (*pFunc)(uint32_t inputA));
//uint32_t Ifx_Jlt(uint32_t A, uint32_t B);
//uint32_t Ifx_Jlt_U(uint32_t A, uint32_t B);
//******************************************************************************
    .global Ifx_J
    .global Ifx_JA
    .global Ifx_JI
    .global Ifx_Jeq
    .global Ifx_Jeq_A
    .global Ifx_Jne
    .global Ifx_Jnz_T
    .global Ifx_Jz_T
    .global Ifx_Jne_A
    .global Ifx_Jnz_A
    .global Ifx_Jz_A
    .global Ifx_Jned
    .global Ifx_Jnei
    .global Ifx_Jge
    .global Ifx_Jge_U
    .global Ifx_Jgez
    .global Ifx_Jgtz
    .global Ifx_Jltz
    .global Ifx_Jnz
    .global Ifx_Jz
    .global Ifx_JL
    .global Ifx_JLA
    .global Ifx_Jlez
    .global Ifx_JLI
    .global Ifx_Jlt
    .global Ifx_Jlt_U


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
	mov inputA, 30
	ji pointerA
    ret

//Jump if Equal
Ifx_Jeq:
	jeq inputA, inputB, ret_3
	mov result, 0
	ret
ret_3:
 	mov result, 1
   	ret

//   Jump if Not Equal
Ifx_Jne:
	jne inputA, inputB, ret_14
	mov result, 0
	ret
ret_14:
 	mov result, 1
   	ret

//    Jump if Not Equal to Zero Bit
Ifx_Jnz_T:
	jnz.t inputA, 1, ret20
	mov result, 0
	ret
ret20:
 	mov result, 1
    ret

//    Jump if Zero Bit
Ifx_Jz_T:
	jz.t inputA, 1, ret23
	mov result, 0
	ret
ret23:
 	mov result, 1
    ret

//Jump if Equal Address
Ifx_Jeq_A:
	jeq.a pointerA, pointerB, ret_4
	mov.aa pointerResult, pointerA
	ret
ret_4:
	mov.aa pointerResult, pointerB
    ret

//Jump if Not Equal Address
Ifx_Jne_A:
	jne.a pointerA, pointerB, ret_15
	mov.aa pointerResult, pointerA
	ret
ret_15:
	mov.aa pointerResult, pointerB
    ret

  //Jump if Not Equal to Zero Address
Ifx_Jnz_A:
	jnz.a pointerA, ret_19
	mov result, 0
	ret
ret_19:
	mov result, 1
    ret

  //Jump if Zero Address
Ifx_Jz_A:
	jz.a pointerA, ret_22
	mov result, 0
	ret
ret_22:
	mov result, 1
    ret

 //Jump if Not Equal and Decrement
Ifx_Jned:
	jned inputA, inputB, ret_16
	mov result, 0
	ret
ret_16:
	mov result, 1
    ret

  //Jump if Not Equal and Increment
Ifx_Jnei:
	jnei inputA, inputB, ret_17
	mov result, 0
	ret
ret_17:
	mov result, 1
    ret

//     Jump if Greater Than or Equal
Ifx_Jge:
	jge inputA, inputB, ret_5
	mov result, 0
	ret
ret_5:
	mov result, 1
    ret

//Jump if Greater Than or Equal Unsigned
Ifx_Jge_U:
	jge.u inputA, inputB, ret_6
	mov result, 0
	ret
ret_6:
	mov result, 1
    ret

//Jump if Greater Than or Equal to Zero (16-bit)
Ifx_Jgez:
	jgez inputA, ret_7
	mov result, 0
	ret
ret_7:
	mov result, 1
    ret

 //Jump if Greater Than Zero (16-bit)
Ifx_Jgtz:
	jgtz inputA, ret_8
	mov result, 0
	ret
ret_8:
	mov result, 1
    ret

 //Jump if Less Than Zero (16-bit)
Ifx_Jltz:
	jltz inputA, ret_13
	mov result, 0
	ret
ret_13:
	mov result, 1
    ret

  //Jump if Not Equal to Zero (16-bit)
Ifx_Jnz:
	jnz inputA, ret_18
	mov result, 0
	ret
ret_18:
	mov result, 1
   ret

 //Jump if Zero (16-bit)
Ifx_Jz:
	jz inputA, ret_21
	mov result, 0
	ret
ret_21:
	mov result, 1
    ret

 //Jump if Less Than or Equal to Zero (16-bit)
Ifx_Jlez:
	jlez inputA, ret_11
	mov result, 0
	ret
ret_11:
	mov result, 1
    ret

//     Jump if Less Than or Equal
Ifx_Jlt:
	jlt inputA, inputB, ret_12
	mov result, 0
	ret
ret_12:
	mov result, 1
    ret

//Jump if Less Than or Equal Unsigned
Ifx_Jlt_U:
	jlt.u inputA, inputB, ret_24
	mov result, 0
	ret
ret_24:
	mov result, 1
    ret

    //Jump and Link
Ifx_JL:
	addi result, inputA, 9
	jl ret_9
	addi result, inputA, 9
	ret
ret_9:
    fret

 //Jump and Link Absolute
Ifx_JLA:
	addi result, inputA, 10
	jla ret_10
	addi result, inputA, 10
    ret
ret_10:
    fret

// Jump and Link Indirect
Ifx_JLI:
	mov inputA, 30
	mov.aa pointerResult, pointerA
	add.a pointerResult, 4
	jli pointerResult
    ret
