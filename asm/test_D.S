//*****************************************************************************
// File: test_D.S
// Compiler: HIGHTEC
//******************************************************************************
//	void Ifx_Debug(void);
//	void Ifx_Nop(void);
//	uint32_t Ifx_Dextr(uint32_t A, uint32_t B, uint32_t pos);
//	uint32_t Ifx_Dextr_I(uint32_t A, uint32_t B);
//	uint32_t Ifx_Disable(void);
//	void Ifx_Dsync(void);
//	uint64_t Ifx_Dvadj(uint64_t inputA64, uint32_t inputC);
//	uint64_t Ifx_Div(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_Div_U(uint32_t inputA, uint32_t inputB);
//	float Ifx_Div_F(float inputA, float inputB);
//	uint64_t Ifx_DivInit(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_DivInit_U(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_DivInit_B(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_DivInit_BU(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_DivInit_H(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_DivInit_HU(uint32_t inputA, uint32_t inputB);
//	uint64_t Ifx_Dvstep(uint64_t inputA64, uint32_t inputC);
//	uint64_t Ifx_Dvstep_U(uint64_t inputA64, uint32_t inputC);
//	void Ifx_Enable(void);
//
//******************************************************************************

//External Reference
    .global Ifx_Debug
    .global Ifx_Dextr
    .global Ifx_Dextr_I
    .global Ifx_Disable
    .global Ifx_Dsync

    .global Ifx_Dvadj
    .global Ifx_Div
    .global Ifx_Div_U
    .global Ifx_Div_F
    .global Ifx_DivInit
    .global Ifx_DivInit_U
    .global Ifx_DivInit_B
    .global Ifx_DivInit_BU
    .global Ifx_DivInit_H
    .global Ifx_DivInit_HU
    .global Ifx_Dvstep
    .global Ifx_Dvstep_U

    .global Ifx_Enable
    .global Ifx_Nop

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

    .align 8

//If the Debug mode is enabled (DBGSR.DE == 1), cause a Debug Event; otherwise execute a NOP.
Ifx_Debug:
	debug
    ret

Ifx_Nop:
	nop
    ret

//	Extract from Double Register
Ifx_Dextr:
	dextr result, inputA, inputB, pos
    ret

Ifx_Dextr_I:
	dextr result, inputA, inputB, 16
    ret

//Disable Interrupts
//Note: DISABLE can only be executed in User-1 mode or Supervisor mode
Ifx_Disable:
	disable result
    ret

//Enable Interrupts
//Note: ENABLE can only be executed in User-1 or Supervisor mode.
Ifx_Enable:
	enable
    ret

//	Synchronize Data
Ifx_Dsync:
	dsync
    ret

//	Divide-Adjust
//	q_sign = E[d][63] ^ D[b][31];
//	x_sign = E[d][63];
//	eq_pos = x_sign & (E[d][63:32] == D[b]);
//	eq_neg = x_sign & (E[d][63:32] == -D[b]);
//
//	if((q_sign & ~eq_neg) | eq_pos) {
//		quotient = E[d][31:0] + 1;
//	} else {
//		quotient = E[d][31:0];
//	}
//
//	if(eq_pos | eq_neg) {
//		remainder = 0;
//	} else {
//		remainder = E[d][63:32];
//	}
//
//	gt = abs(E[d][63:32]) > abs(D[b]);
//	eq = !E[d][63] AND (abs(E[d][63:32] == abs(D[b]));
//	overflow = eq | gt;
//
//	if(overflow) {
//		E[c] = 64'bx;
//	} else {
//		E[c] = {remainder[31:0],quotient[31:0]};
//	}
Ifx_Dvadj:
	dvadj result64, inputA64, inputC
    ret

//Divide
//	dividend = D[a];
//	divisor = D[b];
//	if (divisor == 0) then {
//		if (dividend >= 0) then {
//			quotient = 0x7fffffff;
//			remainder = 0x00000000;
//		} else {
//			quotient = 0x80000000;
//			remainder = 0x00000000;
//		}
//	} else if ((divisor == 0xffffffff) AND (dividend == 0x80000000)) then {
//		quotient = 0x7fffffff;
//		remainder = 0x00000000;
//	} else {
//		remainder = dividend % divisor
//				quotient = (dividend - remainder)/divisor
//	}
//	E[c][31:0] = quotient;
//	E[c][63:32] = remainder;
Ifx_Div:
	div result64, inputA, inputB
	ret

//Divide Unsigned
//	dividend = D[a];
//	divisor = D[b];
//	if (divisor == 0) then {
//		quotient = 0xffffffff;
//		remainder = 0x00000000;
//	} else {
//		remainder = dividend % divisor
//				quotient = (dividend - remainder)/divisor
//	}
//	E[c][31:0] = quotient;
//	E[c][63:32] = remainder;
Ifx_Div_U:
	div.u result64, inputA, inputB
	ret

//Divide Float
//	arg_a = denorm_to_zero(f_real(D[a]);
//	arg_b = denorm_to_zero(f_real(D[b]);
//	if(is_nan(D[a]) OR is_nan(D[b])) then result = QUIET_NAN;
//	else if(is_inf(D[a]) AND is_inf(D[b])) then result = DIV_NAN;
//	else if(is_zero(D[a]) AND is_zero(D[b])) then result = DIV_NAN;
//	else {
//		precise_result = divide(arg_a,arg_b);
//		normal_result = denorm_to_zero(precise_result);
//		rounded_result = ieee754_round(normal_result, PSW.RM);
//		result = ieee754_32bit_format(rounded_result);
//	}
//	D[c] = result[31:0];
Ifx_Div_F:
	add.f inputA, inputA, inputB
	sub.f inputA, inputA, inputB
	div.f result, inputA, inputB
	ret

//Divide-Initialization Word
//	E[c] = sign_ext(D[a]);
Ifx_DivInit:
	dvinit result64, inputA, inputB
	ret

//Divide-Initialization Word Unsigned
//	E[c] = {00000000H, D[a]};
Ifx_DivInit_U:
	dvinit.u result64, inputA, inputB
	ret

//Divide-Initialization Byte
//	quotient_sign = !(D[a][31] == D[b][31]);
//	E[c][63:24] = sign_ext(D[a]);
//	E[c][23:0] = quotient_sign ? 24¡¯b111111111111111111111111 : 24¡¯b0;
Ifx_DivInit_B:
	dvinit.b result64, inputA, inputB
	ret

//Divide-Initialization Byte Unsigned
//	E[c][63:24] = zero_ext(D[a]);
//	E[c][23:0] = 0;
Ifx_DivInit_BU:
	dvinit.bu result64, inputA, inputB
	ret

//Divide-Initialization Half-word
//	quotient_sign = !(D[a][31] == D[b][31];
//	E[c][63:16] = sign_ext(D[a]);
//	E[c][15:0] = quotient_sign ? 16¡¯b1111111111111111 : 16¡¯b0;
Ifx_DivInit_H:
	dvinit.h result64, inputA, inputB
	ret

//Divide-Initialization Half-word Unsigned
//	E[c][63:16] = zero_ext(D[a]);
//	E[c][15:0] = 0;
Ifx_DivInit_HU:
	dvinit.hu result64, inputA, inputB
	ret

//Divide-Step
//	dividend_sign = E[d][63];
//	divisor_sign = D[b][31];
//	quotient_sign = dividend_sign != divisor_sign;
//	addend = quotient_sign ? D[b] : 0 - D[b];
//	dividend_quotient = E[d][31:0];
//	remainder = E[d][63:32];
//	for i = 0 to 7 {
//		remainder = (remainder << 1) | dividend_quotient[31];
//		dividend_quotient <<= 1;
//		temp = remainder + addend;
//		remainder = ((temp < 0) == dividend_sign) ? temp :: remainder;
//		dividend_quotient = dividend_quotient | (((temp < 0) == dividend_sign) ? !quotient_sign : quotient_sign);
//	}
//	E[c] = {remainder[31:0], dividend_quotient[31:0]};
Ifx_Dvstep:
	dvstep result64, inputA64, inputC
	ret

//Divide-Step Unsigned
//	divisor = D[b];
//	dividend_quotient = E[d][31:0];
//	remainder = E[d][63:32];
//	for i = 0 to 7 {
//		remainder = (remainder << 1) | dividend_quotient[31];
//		dividend_quotient <<= 1;
//		temp = remainder - divisor;
//		remainder = (temp < 0) ? remainder : temp;
//		dividend_quotient = dividend_quotient | !(temp < 0);
//	}
//	E[c] = {remainder[31:0], dividend_quotient[31:0]};
Ifx_Dvstep_U:
	dvstep.u result64, inputA64, inputC
	ret

