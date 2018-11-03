/*
 * asm_prototype.h
 *
 *  Created on: Sep 9, 2018
 *      Author: zzr
 */

#ifndef ASM_PROTOTYPE_H_
#define ASM_PROTOTYPE_H_

#include <stdint.h>
#include <stdbool.h>

typedef union union_pack16
{
	uint16_t u16;
	int16_t i16;
	uint8_t u8[2];
	int8_t i8[2];
}pack16;

typedef union union_pack32
{
	uint32_t u32;
	int32_t i32;
	uint16_t u16[2];
	int16_t i16[2];
	uint8_t u8[4];
	int8_t i8[4];
}pack32;

typedef union union_pack64
{
	uint64_t u64;
	int64_t i64;
	uint32_t u32[2];
	int32_t i32[2];
	uint16_t u16[4];
	int16_t i16[4];
	uint8_t u8[8];
	int8_t i8[8];
}pack64;

//Absolute
uint32_t Ifx_Abs_B(uint32_t X);
uint32_t Ifx_Abs_H(uint32_t X);
int32_t Ifx_Abs(int32_t X);
uint32_t Ifx_Absdif_B(uint32_t A, uint32_t B);
uint32_t Ifx_Absdif_H(uint32_t A, uint32_t B);
int32_t Ifx_Absdif(int32_t A, int32_t B);
uint32_t Ifx_AbsS_H(int32_t X);
uint32_t Ifx_AbsdifS_H(uint32_t A, uint32_t B);
uint32_t Ifx_AbsdifS(uint32_t A, uint32_t B);
int32_t Ifx_AbsS(int32_t X);

//Addition
void* Ifx_AddA(void* pA, void* pB);
void* Ifx_AddA_4(void* pA);
void* Ifx_Addih_A(void* pA);
uint32_t Ifx_Add_B(uint32_t A, uint32_t B);
uint32_t Ifx_Add_H(uint32_t A, uint32_t B);
float Ifx_Add_F(float A, float B);
int32_t Ifx_Add(int32_t A, int32_t B);
int32_t Ifx_AddC(int32_t A, int32_t B);
int32_t Ifx_AddI(int32_t A);
int32_t Ifx_AddI_Hi(int32_t A);
int32_t Ifx_Addx(int32_t A, int32_t B);
int32_t Ifx_Addx_I(int32_t A);
int32_t Ifx_AddS(int32_t A, int32_t B);
uint32_t Ifx_AddS_U(uint32_t A, uint32_t B);
int32_t Ifx_AddS_H(int32_t A, int32_t B);
uint32_t Ifx_AddS_HU(uint32_t A, uint32_t B);
void* Ifx_Addsc_A(void* pA, uint32_t scalar);
void* Ifx_Addsc_AT(void* pA, uint32_t scalar);

// AND
uint32_t Ifx_And(uint32_t A, uint32_t B);
uint32_t Ifx_AndI(uint32_t A);
uint32_t Ifx_Andn(uint32_t A, uint32_t B);
uint32_t Ifx_AndnI(uint32_t A);
uint32_t Ifx_And_EQ(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_EQ(uint32_t A, uint32_t sum);
uint32_t Ifx_And_NE(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_NE(uint32_t A, uint32_t sum);
uint32_t Ifx_And_GE(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_GE(uint32_t A, uint32_t sum);
uint32_t Ifx_And_GE_U(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_GE_U(uint32_t A, uint32_t sum);
uint32_t Ifx_And_LT(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_LT(uint32_t A, uint32_t sum);
uint32_t Ifx_And_LT_U(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndI_LT_U(uint32_t A, uint32_t sum);
bool Ifx_And_T(uint32_t A, uint32_t B);
bool Ifx_Andn_T(uint32_t A, uint32_t B);
uint32_t Ifx_AndAnd_T(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndAndn_T(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndNor_T(uint32_t A, uint32_t B, uint32_t sum);
uint32_t Ifx_AndOr_T(uint32_t A, uint32_t B, uint32_t sum);

//B-C Instruction Set
uint32_t Ifx_Bisr(uint32_t A);
uint32_t Ifx_Bmerge(uint32_t A, uint32_t B);
uint64_t Ifx_Bsplit(uint32_t A);
uint32_t Ifx_Cachea_I(void);
uint32_t Ifx_Cachea_W(void);
uint32_t Ifx_Cachea_WI(void);
uint32_t Ifx_Cachei_I(void);
uint32_t Ifx_Cachei_W(void);
uint32_t Ifx_Cachei_WI(void);
uint32_t Ifx_CADD(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_CADD_I(uint32_t A, uint32_t cond);
uint32_t Ifx_CADD_I16(uint32_t A, uint32_t cond);
uint32_t Ifx_CADDN(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_CADDN_I(uint32_t A, uint32_t cond);
uint32_t Ifx_CADDN_I16(uint32_t A, uint32_t cond);
uint32_t Ifx_Call(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Call_A(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Call_I(uint32_t(*p)(uint32_t A, uint32_t B, uint32_t cond),uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Clo(uint32_t A);
uint32_t Ifx_Clo_H(uint32_t A);
uint32_t Ifx_Cls(uint32_t A);
uint32_t Ifx_Cls_H(uint32_t A);
uint32_t Ifx_Clz(uint32_t A);
uint32_t Ifx_Clz_H(uint32_t A);
uint32_t Ifx_Cmov(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Cmov_I(uint32_t A, uint32_t cond);
uint32_t Ifx_Cmovn(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Cmovn_I(uint32_t A, uint32_t cond);
uint32_t Ifx_Cmp_F(float A, float B);
uint32_t Ifx_Csub(uint32_t A, uint32_t B, uint32_t cond);
uint32_t Ifx_Csubn(uint32_t A, uint32_t B, uint32_t cond);

//D Initial Instuction Set
void Ifx_Debug(void);
uint32_t Ifx_Dextr(uint32_t A, uint32_t B, uint32_t pos);
uint32_t Ifx_Dextr_I(uint32_t A, uint32_t B);
uint32_t Ifx_Disable(void);
void Ifx_Dsync(void);
uint64_t Ifx_Dvadj(uint64_t inputA64, uint32_t inputC);
uint64_t Ifx_Div(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_Div_U(uint32_t inputA, uint32_t inputB);
float Ifx_Div_F(float inputA, float inputB);
uint64_t Ifx_DivInit(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_DivInit_U(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_DivInit_B(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_DivInit_BU(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_DivInit_H(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_DivInit_HU(uint32_t inputA, uint32_t inputB);
uint64_t Ifx_Dvstep(uint64_t inputA64, uint32_t inputC);
uint64_t Ifx_Dvstep_U(uint64_t inputA64, uint32_t inputC);

//E and F Initial Instuction Set
uint32_t Ifx_Eq(uint32_t A, uint32_t B);
uint32_t Ifx_Eq_fast(uint32_t A, uint32_t B);

uint32_t Ifx_Eq_B(uint32_t A, uint32_t B);
uint32_t Ifx_Eq_H(uint32_t A, uint32_t B);
uint32_t Ifx_Eq_W(uint32_t A, uint32_t B);
uint32_t Ifx_EqAny_B(uint32_t A, uint32_t B);
uint32_t Ifx_EqAny_H(uint32_t A, uint32_t B);
uint32_t Ifx_Eq_A(void* pointerA, void* pointerB);
uint32_t Ifx_Eq_Z_A(void* pointerA);

int32_t Ifx_Ftoi(float inputA);
int32_t Ifx_Ftoi_Z(float inputA);
uint32_t Ifx_Ftou(float inputA);
uint32_t Ifx_Ftou_Z(float inputA);

uint32_t Ifx_Extr(uint32_t inputA, uint32_t inputB);
uint32_t Ifx_Extr_U(uint32_t inputA, uint32_t inputB);
uint32_t Ifx_Ftoq31(float inputA, uint32_t inputB);
uint32_t Ifx_Ftoq31z(float inputA, uint32_t inputB);
uint32_t Ifx_Fcall(uint32_t A, uint32_t B);
uint32_t Ifx_Fcall_A(uint32_t A, uint32_t B);
uint32_t Ifx_Fcall_I(uint32_t(*p)(uint32_t A, uint32_t B),uint32_t A, uint32_t B);
float Ifx_Itof(int32_t inputA);

uint32_t Ifx_J(uint32_t inputA);
uint32_t Ifx_JA(uint32_t inputA);
uint32_t Ifx_JI(uint32_t (*pFunc)(uint32_t inputA));
uint32_t Ifx_Jeq(uint32_t A, uint32_t B);
void* Ifx_Jeq_A(void* A, void* B);
uint32_t Ifx_Jne(uint32_t A, uint32_t B);
uint32_t Ifx_Jnz_T(uint32_t A);
uint32_t Ifx_Jz_T(uint32_t A);
void* Ifx_Jne_A(void* A, void* B);
void* Ifx_Jnz_A(void* A, void* B);
void* Ifx_Jz_A(void* A, void* B);

uint32_t Ifx_Jned(uint32_t A, uint32_t B);
uint32_t Ifx_Jnei(uint32_t A, uint32_t B);
uint32_t Ifx_Jge(uint32_t A, uint32_t B);
uint32_t Ifx_Jge_U(uint32_t A, uint32_t B);
uint32_t Ifx_Jgez(uint32_t A);
uint32_t Ifx_Jgtz(uint32_t A);
uint32_t Ifx_Jltz(uint32_t A);
uint32_t Ifx_Jnz(uint32_t A);
uint32_t Ifx_Jz(uint32_t A);
uint32_t Ifx_JL(uint32_t A);
uint32_t Ifx_JLA(uint32_t A, uint32_t B);
uint32_t Ifx_Jlez(uint32_t A);
uint32_t Ifx_JLI(uint32_t (*pFunc)(uint32_t inputA));
uint32_t Ifx_Jlt(uint32_t A, uint32_t B);
uint32_t Ifx_Jlt_U(uint32_t A, uint32_t B);

uint32_t* Ifx_LD_A(uint32_t* pA);
int8_t Ifx_LD_B(int8_t* pA);
uint8_t Ifx_LD_BU(uint8_t* pA);
uint64_t Ifx_LD_D(uint64_t* pA);
uint64_t* Ifx_LD_DA(uint64_t* pA);
int16_t Ifx_LD_H(int16_t* pA);
uint16_t Ifx_LD_HU(uint16_t* pA);
uint32_t Ifx_LD_Q(uint32_t* pA);
int32_t Ifx_LD_W(uint32_t* pA);
void Ifx_LDLCX(void);
void Ifx_LDUCX(void);
uint64_t Ifx_LDMST(uint64_t A);
uint32_t* Ifx_LEA(void);
uint32_t Ifx_LOOP(uint32_t A, uint32_t B);
uint32_t Ifx_LOOPU(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_LT(uint32_t A, uint32_t B);
uint32_t Ifx_LT_A(uint32_t* pA, uint32_t* pB);
pack32 Ifx_LT_B(pack32 packA, pack32 packB);
pack32 Ifx_LT_BU(pack32 packA, pack32 packB);
pack32 Ifx_LT_H(pack32 packA, pack32 packB);
pack32 Ifx_LT_HU(pack32 packA, pack32 packB);
uint32_t Ifx_LT_U(uint32_t A, uint32_t B);
int32_t Ifx_LT_W(int32_t A, int32_t B);
uint32_t Ifx_LT_WU(uint32_t A, uint32_t B);

int32_t Ifx_MADD(int32_t A, int32_t B, int32_t C);
int32_t Ifx_MADDS(int32_t A, int32_t B, int32_t C);
uint64_t Ifx_MADD_U(uint64_t A, uint32_t C, uint32_t D);
uint64_t Ifx_MADDS_U(uint64_t A, uint32_t C, uint32_t D);

float Ifx_MADD_F(float A, float B, float C);

uint64_t Ifx_MADD_H(uint64_t A, uint32_t C, uint32_t D);
uint64_t Ifx_MADDS_H(uint64_t A, uint32_t C, uint32_t D);

uint32_t Ifx_MADD_Q(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_MADDS_Q(uint32_t A, uint32_t B, uint32_t C);

//Packed Multiply-Add Q Format Multi-precision
//Packed Multiply-Add Q Format Multi-precision, Saturated
uint64_t Ifx_MADDM_H(uint64_t A, uint32_t C, uint32_t D);
uint64_t Ifx_MADDMS_H(uint64_t A, uint32_t C, uint32_t D);

//Packed Multiply-Add Q Format with Rounding
//Packed Multiply-Add Q Format with Rounding, Saturated
uint32_t Ifx_MADDR_H(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_MADDRS_H(uint32_t A, uint32_t B, uint32_t C);

//Multiply-Add Q Format with Rounding
//Multiply-Add Q Format with Rounding, Saturated
uint32_t Ifx_MADDR_Q(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_MADDRS_Q(uint32_t A, uint32_t B, uint32_t C);

//Packed Multiply-Add/Subtract Q Format
//Packed Multiply-Add/Subtract Q Format Saturated
uint64_t Ifx_MADDSU_H(uint64_t A, uint32_t C, uint32_t D);
uint64_t Ifx_MADDSUS_H(uint64_t A, uint32_t C, uint32_t D);

//Packed Multiply-Add/Subtract Q Format Multi-precision
//Packed Multiply-Add/Subtract Q Format Multi-precision Saturated
uint64_t Ifx_MADDSUM_H(uint64_t A, uint32_t C, uint32_t D);
uint64_t Ifx_MADDSUMS_H(uint64_t A, uint32_t C, uint32_t D);

//Packed Multiply-Add/Subtract Q Format with Rounding
//Packed Multiply-Add/Subtract Q Format with Rounding Saturated
uint32_t Ifx_MADDSUR_H(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_MADDSURS_H(uint32_t A, uint32_t B, uint32_t C);
uint32_t Ifx_Rand32(uint32_t seed, uint32_t B);

void Ifx_Enable(void);

void Ifx_Nop(void);

uint32_t Ifx_Rslcx(void);

#endif /* ASM_PROTOTYPE_H_ */
