//*****************************************************************************
// File: Ifx_And.S
// Compiler: HIGHTEC
//******************************************************************************
// uint32_t Ifx_And(uint32_t A, uint32_t B);
// uint32_t Ifx_AndI(uint32_t A);
// uint32_t Ifx_Andn(uint32_t A, uint32_t B);
// uint32_t Ifx_AndnI(uint32_t A);
// uint32_t Ifx_And_EQ(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_EQ(uint32_t A, uint32_t sum);
// uint32_t Ifx_And_NE(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_NE(uint32_t A, uint32_t sum);
// uint32_t Ifx_And_GE(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_GE(uint32_t A, uint32_t sum);
// uint32_t Ifx_And_GE_U(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_GE_U(uint32_t A, uint32_t sum);
// uint32_t Ifx_And_LT(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_LT(uint32_t A, uint32_t sum);
// uint32_t Ifx_And_LT_U(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndI_LT_U(uint32_t A, uint32_t sum);
// bool Ifx_And_T(uint32_t A, uint32_t B);
// bool Ifx_Andn_T(uint32_t A, uint32_t B);
// uint32_t Ifx_AndAnd_T(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndAndn_T(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndNor_T(uint32_t A, uint32_t B, uint32_t sum);
// uint32_t Ifx_AndOr_T(uint32_t A, uint32_t B, uint32_t sum);

// INPUTS:
// OUTPUT:
//       None.
//
// RETURN:
//
// DESCRIPTION:
//	Bitwise AND
//	Bit Logical AND
//	Bitwise AND-Not
//	Bit Logical AND-Not
//	Equal Accumulating
//	Not Equal Accumulating
//	Greater Than or Equal Accumulating
//	Greater Than or Equal Accumulating Unsigned
//	Less Than Accumulating
//	Less Than Accumulating Unsigned
//	Accumulating Bit Logical AND-AND
//	Accumulating Bit Logical AND-AND-Not
//	Accumulating Bit Logical AND-NOR
//	Accumulating Bit Logical AND-OR
//
// ASSUMPTIONS:
//
// MEMORY NOTE:
//
// REGISTER USAGE:
//            d2, d4,d5
//******************************************************************************

//External Reference
    .global Ifx_And
    .global Ifx_AndI
    .global Ifx_And_T
    .global Ifx_Andn
    .global Ifx_AndnI
    .global Ifx_Andn_T
    .global Ifx_And_EQ
    .global Ifx_AndI_EQ
    .global Ifx_And_NE
    .global Ifx_AndI_NE
    .global Ifx_And_GE
    .global Ifx_AndI_GE
    .global Ifx_And_GE_U
    .global Ifx_AndI_GE_U
    .global Ifx_And_LT
    .global Ifx_AndI_LT
    .global Ifx_And_LT_U
    .global Ifx_AndI_LT_U
    .global Ifx_AndAnd_T
    .global Ifx_AndAndn_T
    .global Ifx_AndNor_T
    .global Ifx_AndOr_T

#define     inputA       %d4
#define     inputB       %d5
#define     inputC       %d6
#define     result    	%d2

//Executable Code
    .align 8
//	Bitwise AND RR
Ifx_And:
	and	result, inputA, inputB
    ret

//	Bitwise AND RC
Ifx_AndI:
	and	result, inputA, 0xF
    ret

//	Bit Logical AND
Ifx_And_T:
	and.t result, inputA, 0, inputB, 0
    ret

//	Bitwise AND-Not RR
Ifx_Andn:
	andn	result, inputA, inputB
    ret

//	Bitwise AND-Not RC
Ifx_AndnI:
	andn	result, inputA, 0xF
    ret

//	Bit Logical AND-Not
Ifx_Andn_T:
	andn.t result, inputA, 0, inputB, 0
    ret

//	Equal Accumulating	RR
Ifx_And_EQ:
	and.eq	inputC, inputA, inputB
	mov result, inputC
    ret

//	Equal Accumulating	RC
Ifx_AndI_EQ:
	and.eq	inputB, inputA, 0xF
 	mov result, inputB
   ret

//	Not Equal Accumulating	RR
Ifx_And_NE:
	and.ne	inputC, inputA, inputB
	mov result, inputC
    ret

//	Not Equal Accumulating	RC
Ifx_AndI_NE:
	and.ne	inputB, inputA, 0xF
 	mov result, inputB
    ret

//	Greater Than or Equal Accumulating	RR
Ifx_And_GE:
	and.ge	inputC, inputA, inputB
	mov result, inputC
	ret

//	Greater Than or Equal Accumulating	RC
Ifx_AndI_GE:
 	and.ge	inputB, inputA, 0xF
 	mov result, inputB
    ret

//	Greater Than or Equal Accumulating Unsigned	RR
Ifx_And_GE_U:
	and.ge.u	inputC, inputA, inputB
	mov result, inputC
    ret

//	Greater Than or Equal Accumulating Unsigned	RC
Ifx_AndI_GE_U:
 	and.ge.u	inputB, inputA, 0xF
 	mov result, inputB
    ret

//	Less Than Accumulating	RR
Ifx_And_LT:
	and.lt	inputC, inputA, inputB
	mov result, inputC
    ret

//	Less Than Accumulating	RC
Ifx_AndI_LT:
 	and.lt	inputB, inputA, 0xF
 	mov result, inputB
    ret

//	Less Than Accumulating Unsigned	RR
Ifx_And_LT_U:
	and.lt.u	inputC, inputA, inputB
	mov result, inputC
    ret

//	Less Than Accumulating Unsigned	RC
Ifx_AndI_LT_U:
 	and.lt.u	inputB, inputA, 0xF
 	mov result, inputB
    ret

//	Accumulating Bit Logical AND-AND
Ifx_AndAnd_T:
   and.and.t inputC, inputA, 1, inputB, 1
   mov result, inputC
   ret

//	Accumulating Bit Logical AND-AND-Not
Ifx_AndAndn_T:
	and.andn.t inputC, inputA, 1, inputB, 1
   	mov result, inputC
    ret

//	Accumulating Bit Logical AND-NOR
Ifx_AndNor_T:
   and.nor.t inputC, inputA, 1, inputB, 1
   mov result, inputC
   ret

//	Accumulating Bit Logical AND-OR
Ifx_AndOr_T:
   and.or.t inputC, inputA, 1, inputB, 1
   mov result, inputC
   ret
