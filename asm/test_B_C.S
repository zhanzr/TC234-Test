//*****************************************************************************
// File: test_B_C.S
// Compiler: HIGHTEC
//******************************************************************************
// uint32_t Ifx_Bisr(uint32_t A);
// uint32_t Ifx_Bmerge(uint32_t A, uint32_t B);
// uint64_t Ifx_Bsplit(uint32_t A);
// uint32_t Ifx_Cachea_I(void);
// uint32_t Ifx_Cachea_W(void);
// uint32_t Ifx_Cachea_WI(void);
// uint32_t Ifx_Cachei_I(void);
// uint32_t Ifx_Cachei_W(void);
// uint32_t Ifx_Cachei_WI(void);
// uint32_t Ifx_CADD(uint32_t A, uint32_t B, uint32_t cond);
// uint32_t Ifx_CADD_I(uint32_t A, uint32_t B);
// uint32_t Ifx_CADD_I16(uint32_t A, uint32_t B);
// uint32_t Ifx_CADDN(uint32_t A, uint32_t B, uint32_t cond);
// uint32_t Ifx_CADDN_I(uint32_t A, uint32_t B);
// uint32_t Ifx_CADDN_I16(uint32_t A, uint32_t B);
// void Ifx_Call(void);
// void Ifx_Call_A(void);
// void Ifx_Call_I(void(*p)(void));
// uint32_t Ifx_Clo(uint32_t A);
// uint32_t Ifx_Clo_H(uint32_t A);
// uint32_t Ifx_Cls(uint32_t A);
// uint32_t Ifx_Cls_H(uint32_t A);
// uint32_t Ifx_Clz(uint32_t A);
// uint32_t Ifx_Clz_H(uint32_t A);
// uint32_t Ifx_Cmov(uint32_t A, uint32_t B);
// uint32_t Ifx_Cmov_I(uint32_t A);
// uint32_t Ifx_Cmovn(uint32_t A, uint32_t B);
// uint32_t Ifx_Cmovn_I(uint32_t A);
// uint32_t Ifx_Cmp_F(uint32_t A, uint32_t B);
// uint32_t Ifx_Csub(uint32_t A, uint32_t B);
// uint32_t Ifx_Csubn(uint32_t A, uint32_t B);

// INPUTS:
// OUTPUT:
//       None.
//
// RETURN:
//
//******************************************************************************

//External Reference
    .global Ifx_Bisr
    .global Ifx_Bmerge
    .global Ifx_Bsplit
    .global Ifx_Cachea_I
    .global Ifx_Cachea_W
    .global Ifx_Cachea_WI
    .global Ifx_Cachei_I
    .global Ifx_Cachei_W
    .global Ifx_Cachei_WI
    .global Ifx_CADD
    .global Ifx_CADD_I
    .global Ifx_CADD_I16
    .global Ifx_CADDN
    .global Ifx_CADDN_I
    .global Ifx_CADDN_I16
    .global Ifx_Call
	.global Ifx_Call_A
  	.global Ifx_Call_I
  	.global Ifx_Clo
  	.global Ifx_Clo_H
  	.global Ifx_Cls
  	.global Ifx_Cls_H
  	.global Ifx_Clz
  	.global Ifx_Clz_H
    .global Ifx_Cmov
    .global Ifx_Cmov_I
    .global Ifx_Cmovn
    .global Ifx_Cmovn_I
    .global Ifx_Cmp_F
    .global Ifx_Csub
    .global Ifx_Csubn

//    .extern TestFunc

#define     inputA       %d4
#define     inputB       %d5
#define     tmpD1       %d1
#define     tmpD15       %d15
#define     inputC       %d6
#define     cond       %d6
#define     result    	%d2
#define     result64    	%e2
#define     pointerA       %a4

//Executable Code
    .align 8
//	Begin Interrupt Service Routine
Ifx_Bisr:
	bisr 126
    ret

//	Bit Merge
Ifx_Bmerge:
	bmerge	result, inputA, inputB
    ret

//	Bit Split
Ifx_Bsplit:
	bsplit result64, inputA
    ret

//	Cache Address, Invalidate
Ifx_Cachea_I:
//	cachea.i [a3]4
//	cachea.i [+a3]4
//	cachea.i [a3+]4
//	cachea.i [a4/a5+c]4
//	cachea.i [a4/a5+r]
    ret

//	Cache Address, Writeback
Ifx_Cachea_W:
//	cachea.w [a3]4
//	cachea.w [+a3]4
//	cachea.w [a3+]4
//	cachea.w [a4/a5+c]4
//	cachea.w [a4/a5+r]
    ret

//	Cache Address, Writeback and Invalidate
Ifx_Cachea_WI:
//	cachea.wi [a3]4
//	cachea.wi [+a3]4
//	cachea.wi [a3+]4
//	cachea.wi [a4/a5+c]4
//	cachea.wi [a4/a5+r]
    ret

  //	Cache Index, Invalidate
Ifx_Cachei_I:
// cachei.i [a3]4
// cachei.i [+a3]4
// cachei.i [a3+]4
    ret

//	Cache Index, Writeback
Ifx_Cachei_W:
// cachei.w [a3]4
// cachei.w [+a3]4
// cachei.w [a3+]4
    ret

//	Cache Index, Writeback and Invalidate
Ifx_Cachei_WI:
// cachei.wi [a3]4
// cachei.wi [+a3]4
// cachei.wi [a3+]4
    ret

Ifx_CADD:
    cadd result, cond, inputA, inputB
	ret

Ifx_CADD_I:
    cadd result, inputA, inputB, 126
	ret

Ifx_CADD_I16:
	mov tmpD1, inputA
	mov tmpD15, inputB
	cadd tmpD1, tmpD15, 6
	mov result, tmpD1
	ret

Ifx_CADDN:
    caddn result, cond, inputA, inputB
	ret

Ifx_CADDN_I:
    caddn result, inputA, inputB, 126
	ret

Ifx_CADDN_I16:
	mov tmpD1, inputA
	mov tmpD15, inputB
	caddn tmpD1, tmpD15, 6
	mov result, tmpD1
	ret

Ifx_Call:
	call Ifx_CADDN_I16
	ret

Ifx_Call_A:
	calla Ifx_CADDN_I16
	ret

//	Count Leading Ones
Ifx_Clo:
	clo result, inputA
	ret

//	Count Leading Ones in Packed Half-words
Ifx_Clo_H:
	clo.h result, inputA
	ret

//	Count Leading Signs
Ifx_Cls:
	cls result, inputA
	ret

//	Count Leading Signs in Packed Half-words
Ifx_Cls_H:
	cls.h result, inputA
	ret

//	Count Leading Zeros
Ifx_Clz:
	clz result, inputA
	ret

//	Count Leading Zeros in Packed Half-words
Ifx_Clz_H:
	clz.h result, inputA
	ret

// Conditional Move (16-bit)
Ifx_Cmov:
	mov tmpD1, inputA
	mov tmpD15, inputB
	cmov tmpD1, tmpD15, result
	mov result, tmpD1
	ret

Ifx_Cmov_I:
	mov tmpD1, inputA
	mov tmpD15, inputB
	cmov tmpD1, tmpD15, 6
	mov result, tmpD1
	ret

//	Conditional Move-Not (16-bit)
Ifx_Cmovn:
	mov tmpD1, inputA
	mov tmpD15, inputB
	cmovn tmpD1, tmpD15, result
	mov result, tmpD1
	ret

Ifx_Cmovn_I:
	mov tmpD1, inputA
	mov tmpD15, inputB
	cmovn tmpD1, tmpD15, 6
	mov result, tmpD1
	ret

//	Conditional Subtract
Ifx_Csub:
	csub result, inputA, inputB, cond
	ret

//	Conditional Subtract-Not
Ifx_Csubn:
	csubn result, inputA, inputB, cond
	ret

// Compare Float
Ifx_Cmp_F:
	cmp.f result, inputA, inputB
	ret

