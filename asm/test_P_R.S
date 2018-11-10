//*****************************************************************************
// File: test_P_R.S
// Compiler: HIGHTEC
//******************************************************************************
//float Ifx_PACK(uint64_t A64, int32_t sign);
//uint64_t Ifx_UNPACK(float fA);
//uint32_t Ifx_PARITY(uint32_t A);
//float Ifx_Q31TOF(uint32_t Q, uint32_t exp);
//float Ifx_QSEED_F(float fA);
//uint32_t Ifx_RESTORE(void);
//uint32_t Ifx_RET(void);
//uint32_t Ifx_RFE(void);
//uint32_t Ifx_RFM(void);
//uint32_t Ifx_RSLCX(void);
//uint32_t Ifx_RSTV(void);
//int32_t Ifx_RSUB(int32_t A);
//int32_t Ifx_RSUBS(int32_t A);
//uint32_t Ifx_RSUBS_U(uint32_t A);
//******************************************************************************
 	.global Ifx_PACK
  	.global Ifx_UNPACK
	.global Ifx_PARITY
 	.global Ifx_Q31TOF
 	.global Ifx_QSEED_F
 	.global Ifx_RESTORE
 	.global Ifx_RET
 	.global Ifx_RFE
 	.global Ifx_RFM
 	.global Ifx_RSLCX
 	.global Ifx_RSTV
 	.global Ifx_RSUB
 	.global Ifx_RSUBS
 	.global Ifx_RSUBS_U

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
#define	pointerA64       %p4
#define	pointerResult      %a2
#define	pointerResult64      %p2

    .align 8
//	PACK
//	UNPACK
//	PARITY
//	Q31TOF
//	QSEED.F
//	RESTORE
//	RET
//	RFE
//	RFM
//	RSLCX
//	RSTV
//	RSUB
//	RSUBS
//	RSUBS.U

//Take the data register pair E[d] and bit 31 of data register D[a] and pack them into an IEEE-754 single precision
//floating point format number, in data register D[c]. The odd register E[d][63:32], holds the unbiased exponent. The
//even register E[d][31:0], holds the normalised mantissa in a fractional 1.31 format. Bit 31 of data register D[a] holds
//the sign bit.
//To compute the floating point format number, the input number is first checked for special cases: Infinity, NAN,
//Overflow, Underflow and Zero. If the input number is not one of these special cases, it is either a normal or
//denormal number. In both cases, rounding of the input number is performed. First an intermediate biased
//exponent is calculated, by adding 128 to the unpacked exponent for normal numbers and set to zero for denormal
//numbers, and inserted into bits [30:23] of the intermediate result. Bits [30:8] of E[d] are inserted into bits [22:0] of
//the intermediate result. A round flag is calculated from bits [8:0] of E[d] using the IEEE-754 Round-to-Nearest
//rounding definition, with the PSW.C field acting as an additional sticky bit. If the round flag is set, the intermediate
//result is incremented by one. Bits [30:0] of the intermediate result are then inserted into bits [30:0] of D[c]. In all
//cases, bit 31 from D[a] is copied into bit 31 of D[c]. The special cases are handled as described below.

//PACK D[c], E[d], D[a] (RRR)
Ifx_PACK:
	pack result, inputA64, inputC
	ret

//Unpack Floating Point
Ifx_UNPACK:
	unpack result64, inputA
	ret

//Compute the four byte parity bits of data register D[a]. Put each byte parity bit into every 8th bit of the data register
//D[c] and then clear the remaining bits of D[c]. A byte parity bit is set to one if the number of ones in a byte is an
//odd number.

//PARITY D[c], D[a] (RR)
Ifx_PARITY:
	PARITY result, inputA
	ret

//Fraction to Floating-point
//Converts the D[a] from Q31 fraction format to floating-point format, then adds D[b] to the exponent and stores the
//resulting value in D[c]. The exponent adjustment is a 9-bit two’s complement number taken from D[b][8:0], with a
//value in the range [-256, 255]. D[b][31:9] is ignored. Q31 fraction format is a 32-bit two’s complement format which
//represents a value in the range [-1,1).
//• Bit 31 represents -1
//• Bit 30 represents +1/2
//• Bit 29 represents +1/4
//• etc.
//Q31TOF D[c], D[a], D[b] (RR)
Ifx_Q31TOF:
	Q31TOF result, inputA, inputB
	ret

//Inverse Square Root Seed
//An approximation of the reciprocal of the square root of D[a] is stored in D[c]. The accuracy of the result is no less
//than 6.75 bits, and therefore always within ±1% of the accurate result.
//The operand and result are floating-point numbers. If the operand is ±0 then the result will be the appropriately
//signed ?. If the operand is a NaN (quiet or signalling), then the return result will be the quiet NaN 7FC00000H.
//This instruction can be used to implement a floating-point square root function in software using the NewtonRaphson iterative method.
//QSEED.F D[c], D[a] (RR)
Ifx_QSEED_F:
	QSEED.F result, inputA
	ret

//Restore the Interrupt Enable bit (ICR.IE) to the value saved in D[a][0].
//RESTORE can only be executed in User-1 or Supervisor mode.
//RESTORE D[a] (SYS)
Ifx_RESTORE:
	RESTORE result
	ret

//Return from Call
//
//Return from a function that was invoked with a CALL instruction. The return address is in register A[11] (return
//address). The caller’s upper context register values are restored as part of the return operation.
Ifx_RET:
	ret

//Return From Exception

//Return from an interrupt service routine or trap handler to the task whose saved upper context is specified by the
//contents of the Previous Context Information register (PCXI). The contents are normally the context of the task
//that was interrupted or that took a trap. However in some cases Task Management software may have altered the
//contents of the PCXI register to cause another task to be dispatched.
//The return PC value is taken from register A[11] (register address) in the current context. In parallel with the jump
//to the return PC address, the upper context registers and PSW in the saved context are restored.
Ifx_RFE:
	rfe
	//ret

//Return From Monitor
//Description
//Note: The RFM instruction can only be executed in Supervisor mode.
Ifx_RFM:
	rfm
	//ret

//Restore Lower Context
//Description
//Load the contents of the memory block pointed to by the PCX field in PCXI into registers A[2] to A[7], D[0] to D[7],
//A[11] (return address), and PCXI. This operation restores the register contents of a previously saved lower
//context.
Ifx_RSLCX:
	RSLCX
	ret

//Reset Overflow Bits
//Reset overflow status flags in the Program Status Word (PSW).
Ifx_RSTV:
	RSTV
	ret

//Reverse-Subtract
Ifx_RSUB:
	RSUB result, inputA, 0
	ret

//Reverse-Subtract with Saturation
Ifx_RSUBS:
	RSUBS result, inputA, 0
	ret

//Reverse-Subtract Unsigned with Saturation
Ifx_RSUBS_U:
	RSUBS.U result, inputA, 0
	ret
