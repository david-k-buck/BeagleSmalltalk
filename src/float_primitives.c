// float_primitives.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include "object.h"

#define PRIM_FLOAT_PLUS 41
#define PRIM_FLOAT_MINUS 42
#define PRIM_FLOAT_LESS_THAN 43
#define PRIM_FLOAT_GREATER_THAN 44
#define PRIM_FLOAT_EQUALS 45
#define PRIM_FLOAT_TIMES 49
#define PRIM_FLOAT_DIVIDE 50
#define PRIM_FLOAT_TRUNCATED 51
#define PRIM_FLOAT_SQRT 52
#define PRIM_FLOAT_SIN 53
#define PRIM_FLOAT_COS 54
#define PRIM_FLOAT_TAN 55
#define PRIM_FLOAT_ATAN2 56
#define PRIM_FLOAT_LOG 57
#define PRIM_FLOAT_ARCSIN 58
#define PRIM_FLOAT_ARCCOS 59
#define PRIM_FLOAT_EXP 65

doubleConverter dc;

void primFloatPlus()
{
	double receiver = stFloatToC(getReceiver());
	double result;
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (asOop(receiver));
		return;
	}

	result = receiver + stFloatToC(arg1);
	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatLessThan()
{
	double receiver = stFloatToC(getReceiver());
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (asOop(receiver));
		return;
	}

	push (cIntToST(0));
	if (receiver < stFloatToC(arg1))
		push (ST_TRUE);
	else
		push (ST_FALSE);
}

void primFloatGreaterThan()
{
	double receiver = stFloatToC(getReceiver());
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (asOop(receiver));
		return;
	}

	push (cIntToST(0));
	if (receiver > stFloatToC(arg1))
		push (ST_TRUE);
	else
		push (ST_FALSE);
}

void primFloatEquals()
{
    double receiver = stFloatToC(getReceiver());
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (asOop(receiver));
		return;
	}

	push (cIntToST(0));
	if (receiver == stFloatToC(arg1))
		push (ST_TRUE);
	else
		push (ST_FALSE);
}

void primFloatMinus()
{
	double receiver = stFloatToC(getReceiver());
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (asOop(receiver));
		return;
	}

    double result = receiver - stFloatToC(arg1);

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatTimes()
{
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (getReceiver());
		return;
	}

    double result = stFloatToC(getReceiver()) * stFloatToC(arg1);

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatDivide()
{
	oop arg1 = getLocal( 0);

	if (!isFloat(arg1)) {
		push (cIntToST(1));
		push (getReceiver());
		return;
	}

	double result = stFloatToC(getReceiver()) / stFloatToC(arg1);

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatTruncated()
{
	int64_t value;

	value = (int64_t) stFloatToC(getReceiver());

	push (cIntToST(0));
	push (cIntToST(value));
}

void primFloatSqrt()
{
	oop receiverOop = getReceiver();
	double receiver = stFloatToC(receiverOop);
	double result = sqrt(receiver);
	oop oopResult = cFloatToST(result);

	push (cIntToST(0));
	push (oopResult);
}

void primFloatSin()
{
	double result = sin(stFloatToC(getReceiver()));

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatCos()
{
	double result = cos(stFloatToC(getReceiver()));

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatTan()
{
	double result = tan(stFloatToC(getReceiver()));

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatLog()
{
	double result = log(stFloatToC(getReceiver()));

	push (cIntToST(0));
	push (cFloatToST(result));
}

void primFloatATan2()
{
    double result = atan2(stFloatToC(getReceiver()), stFloatToC((getLocal(0))));

    push (cIntToST(0));
    push (cFloatToST(result));
}

void primFloatArcsin()
{
    double result = asin(stFloatToC(getReceiver()));

    push (cIntToST(0));
    push (cFloatToST(result));
}

void primFloatArccos()
{
    double result = acos(stFloatToC(getReceiver()));

    push (cIntToST(0));
    push (cFloatToST(result));
}

void primFloatExp()
{
    double result = exp(stFloatToC(getReceiver()));

    push (cIntToST(0));
    push (cFloatToST(result));
}

void initializeFloatPrimitives()
{
	primitiveTable[PRIM_FLOAT_PLUS] = primFloatPlus;
	primitiveTable[PRIM_FLOAT_MINUS] = primFloatMinus;
	primitiveTable[PRIM_FLOAT_TIMES] = primFloatTimes;
	primitiveTable[PRIM_FLOAT_DIVIDE] = primFloatDivide;
	primitiveTable[PRIM_FLOAT_LESS_THAN] = primFloatLessThan;
	primitiveTable[PRIM_FLOAT_GREATER_THAN] = primFloatGreaterThan;
	primitiveTable[PRIM_FLOAT_EQUALS] = primFloatEquals;
	primitiveTable[PRIM_FLOAT_TRUNCATED] = primFloatTruncated;
	primitiveTable[PRIM_FLOAT_SQRT] = primFloatSqrt;
	primitiveTable[PRIM_FLOAT_SIN] = primFloatSin;
	primitiveTable[PRIM_FLOAT_COS] = primFloatCos;
	primitiveTable[PRIM_FLOAT_TAN] = primFloatTan;
    primitiveTable[PRIM_FLOAT_ATAN2] = primFloatATan2;
    primitiveTable[PRIM_FLOAT_LOG] = primFloatLog;
    primitiveTable[PRIM_FLOAT_ARCSIN] = primFloatArcsin;
    primitiveTable[PRIM_FLOAT_ARCCOS] = primFloatArccos;
    primitiveTable[PRIM_FLOAT_EXP] = primFloatExp;
}
