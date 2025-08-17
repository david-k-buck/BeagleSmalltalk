// integer_primitives.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "object.h"

#define PRIM_SMALLINTEGER_PLUS 1
#define PRIM_SMALLINTEGER_MINUS 2
#define PRIM_SMALLINTEGER_LESS_THAN 3
#define PRIM_SMALLINTEGER_GREATER_THAN 4
#define PRIM_SMALLINTEGER_LESS_THAN_OR_EQUAL 5
#define PRIM_SMALLINTEGER_GREATER_THAN_OR_EQUAL 6
#define PRIM_SMALLINTEGER_EQUAL 7
#define PRIM_SMALLINTEGER_NOT_EQUAL 8
#define PRIM_SMALLINTEGER_TIMES 9
#define PRIM_SMALLINTEGER_DIVIDE 10
#define PRIM_SMALLINTEGER_MODULO 11
#define PRIM_SMALLINTEGER_INT_DIVIDE 12
#define PRIM_SMALLINTEGER_AS_FLOAT 13
#define PRIM_BITSHIFT 14

#define PRIM_AS_LARGEINTEGER 20
#define PRIM_LARGEINTEGER_PLUS 21
#define PRIM_LARGEINTEGER_MINUS 22
#define PRIM_LARGEINTEGER_LESS_THAN 23
#define PRIM_LARGEINTEGER_GREATER_THAN 24
#define PRIM_LARGEINTEGER_LESS_THAN_OR_EQUAL 25
#define PRIM_LARGEINTEGER_GREATER_THAN_OR_EQUAL 26
#define PRIM_LARGEINTEGER_EQUAL 27
#define PRIM_LARGEINTEGER_NOT_EQUAL 28
#define PRIM_LARGEINTEGER_TIMES 29
#define PRIM_LARGEINTEGER_DIVIDE 30
#define PRIM_LARGEINTEGER_MODULO 31
#define PRIM_LARGEINTEGER_INT_DIVIDE 32
#define PRIM_LARGEINTEGER_DIVIDE_WITH_REMAINDER 33
#define PRIM_LARGEINTEGER_TIMES_FAST 34
#define PRIM_LARGEINTEGER_AS_FLOAT 35

#define PRIM_BIT_AND 90
#define PRIM_BIT_OR 91
#define PRIM_BIT_XOR 92
#define PRIM_BIT_INVERT 93


void primSmallIntegerPlus()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal(0);
	oop result;

    if (!isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }
    
	int64_t intResult = stIntToC((int64_t)receiver) + stIntToC((int64_t)arg);
	int64_t carry = intResult & (int64_t)0xF000000000000000L;
	if ((carry == 0) || (carry == (int64_t)0xF000000000000000L))
		result = cIntToST((oop)intResult);
	else
		result = asSumLargeInteger(intResult);

	push (cIntToST(0));
	push (result);
}

void primSmallIntegerMinus()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);
	oop result;

    if (!isSmallInteger(receiver) || !isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }
 
	int64_t x = stIntToC(receiver);
	int64_t y = stIntToC(arg);
	int64_t intResult = x - y;

	int64_t borrow = intResult & (int64_t)0xF000000000000000L;
	if ((borrow == 0) || (borrow == (int64_t)0xF000000000000000L))
		result = cIntToST(intResult);
	else
		result = asSumLargeInteger(intResult);


	push (cIntToST(0));
	push (result);
}

void primSmallIntegerLessThan()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) < stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerGreaterThan()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) > stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerLessThanOrEqual()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) <= stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerGreaterThanOrEqual()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) >= stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerEqual()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) == stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerNotEqual()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);

	if (!isSmallInteger(arg)) {
		push (cIntToST(1));
		push (receiver);
		return;
	}

	push (cIntToST(0));
	push (stIntToC(receiver) != stIntToC(arg) ? ST_TRUE : ST_FALSE);
}

void primSmallIntegerTimes()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);
	oop result;
	
    if (!isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }
    
	int64_t x = stIntToC(receiver);
	int64_t y = stIntToC(arg);

	if ((ABS(x) < 0x40000000L) && (ABS(y) < 0x40000000L)) {
		result = cIntToST(x * y);
	}
	else {	
		result = largeIntegerTimes(smallToLargeInteger(receiver), smallToLargeInteger(arg));
	}

	push (cIntToST(0));
	push (result);
}

void primSmallIntegerDivide()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);
	oop result;

    if (!isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }

	if (stIntToC(arg) == 0) {
        push (cIntToST(1));
        push (receiver);
        return;
	}
  
    result = cIntToST(stIntToC(receiver) / stIntToC(arg));

	push (cIntToST(0));
	push (result);
}

void primSmallIntegerModulo()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);
	oop result;

    if (!isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }

	if (stIntToC(arg) == 0) {
        push (cIntToST(1));
        push (receiver);
        return;
	}

    result = cIntToST(stIntToC(receiver) % stIntToC(arg));

	push (cIntToST(0));
	push (result);
}

void primSmallIntegerIntDivide()
{
	oop receiver = (oop) getReceiver();
	oop arg = (oop) getLocal( 0);
	oop result;

    if (!isSmallInteger(arg)) {
        push (cIntToST(1));
        push (receiver);
        return;
    }
    
	if (stIntToC(arg) == 0) {
        push (cIntToST(1));
        push (receiver);
        return;
	}
  
    result = cIntToST(stIntToC(receiver) / stIntToC(arg));

	push (cIntToST(0));
	push (result);
}

void primSmallIntegerAsFloat()
{
	oop receiver = (oop) getReceiver();
	double result;
	oop oopResult;

	result = (double)(stIntToC(receiver));

	push (cIntToST(0));
	oopResult = cFloatToST(result);
	push (oopResult);
}

void primBitShift()
{
	int64_t receiver = stIntToC(getReceiver());
	int64_t shift = stIntToC(getLocal( 0));

	push (cIntToST(0));
	if (shift > 0)
		push (cIntToST(receiver << shift));
	else
		push (cIntToST(receiver >> (0 - shift)));
}

uint64_t largeComponentAt(oop x, uint64_t index)
{
	oop largeIntegerArray = asLargeInteger(x)->bytes;
	if (((index / 2) + 1) > indexedObjectSize(largeIntegerArray))
		return 0L;
	return ((uint32_t *)asObjectHeader(largeIntegerArray)->bodyPointer)[index];
}

uint64_t computeLargeComponentSize(oop x)
{
	oop byteArray = asLargeInteger(x)->bytes;
	uint64_t byteArraySize = totalObjectSize(byteArray);
	int32_t lastComponent = (int32_t)byteArraySize * 2 - 1;
	while ((lastComponent > 0) && (largeComponentAt(x, lastComponent) == 0))
		lastComponent--;
	
	return lastComponent + 1;
}

void largeComponentAtPut(oop x, uint64_t index, uint64_t value)
{
	oop largeIntegerArray = asLargeInteger(x)->bytes;
	if (((index / 2) + 1) > indexedObjectSize(largeIntegerArray))
		return;
	((uint32_t *)asObjectHeader(largeIntegerArray)->bodyPointer)[index] = value;

	if (value == 0) {
		if ((index + 1) == largeComponentSize(x)) {
			asLargeInteger(x)->componentSize = cIntToST(computeLargeComponentSize(x));
		}
	} else {
		if ((index + 1) > largeComponentSize(x))
			asLargeInteger(x)->componentSize = cIntToST(index + 1);
	}
}

void largeComponentAtAdd (oop x, uint64_t index, uint64_t value)
{
	uint32_t carry = 0;
	uint64_t i;
	uint64_t componentSize = totalObjectSize(instVarAtInt(x, 0))*2;
	uint64_t component = value;
	
	for (i = index; i < componentSize + 1; i++) {
		uint64_t sum;
		sum = ((uint64_t) largeComponentAt(x, i)) + (component & 0xFFFFFFFFL) + carry;
		largeComponentAtPut(x, i, sum & 0xFFFFFFFFL);
		component = component >> 32;
		carry = sum >> 32;
		if ((component == 0) && (carry == 0)) break;
	}
}

/*
uint64_t largeComponentSize(oop x)
{
	return largeComponentSize2(x);
	oop byteArray = asLargeInteger(x)->bytes;
	uint64_t byteArraySize = totalObjectSize(byteArray);
	int32_t lastComponent = byteArraySize * 2 - 1;
	while ((lastComponent > 0) && (largeComponentAt(x, lastComponent) == 0))
		lastComponent--;
	
	return lastComponent + 1;

}
*/

/*uint64_t largeComponentSize(oop x)
{
	oop byteArray = asLargeInteger(x)->bytes;
	uint64_t byteArraySize = totalObjectSize(byteArray);
	uint32_t result;
	
	if (((uint32_t *)(asObjectHeader(byteArray)->bodyPointer))[byteArraySize * 2 - 1] == 0)
		result = byteArraySize * 2 - 1;
	else
		result = byteArraySize * 2;
	
	return result;
}
*/

int isZero(oop x)
{
	if (largeComponentSize(x) > 1)
		return FALSE;
	
	if (largeComponentAt(x, 0) == 0)
		return TRUE;

	return FALSE;
}

oop largeIntegerReduce(oop x)
{
	if (isSmallInteger(x))
		return x;

	DEFINE_LOCALS;
	DEFINE_LOCAL(x);
	SET_LOCAL(x, x);
	DEFINE_LOCAL(largeIntegerArray);
	SET_LOCAL(largeIntegerArray, asLargeInteger(x)->bytes);
	DEFINE_LOCAL(newArray);
	
	int32_t lastDigit, objectSize;
	lastDigit = objectSize = totalObjectSize(GET_LOCAL(largeIntegerArray));
	
	while ((lastDigit > 1) && (((uint64_t) instVarAtInt(GET_LOCAL(largeIntegerArray), lastDigit - 1)) == 0))
		lastDigit--;

	if ((lastDigit == 1) && (instVarAtInt(GET_LOCAL(largeIntegerArray), 0) < 0x1000000000000000L)) {
		oop result = cIntToST(instVarAtInt(GET_LOCAL(largeIntegerArray), 0) * ((classOf(x) == ST_LARGE_POSITIVE_INTEGER_CLASS)?1:-1));
		FREE_LOCALS;
		return result;
	}

	if (lastDigit == objectSize) {
		oop result = GET_LOCAL(x);
		FREE_LOCALS;
		return result;
	}

	SET_LOCAL(newArray, newInstanceOfClass (ST_BYTE_ARRAY_CLASS, lastDigit * sizeof(oop), EdenSpace));
	instVarAtIntPut(GET_LOCAL(x), 0, GET_LOCAL(newArray));

	int32_t i;
	for (i = 0; i < lastDigit; i++)
		basicInstVarAtIntPut(GET_LOCAL(newArray), i, instVarAtInt(GET_LOCAL(largeIntegerArray), i));

	oop result = GET_LOCAL(x);
	asLargeInteger(GET_LOCAL(x))->componentSize = cIntToST(computeLargeComponentSize(GET_LOCAL(x)));
	FREE_LOCALS;

	return result;
}

int largeIntegerABSCompare(oop x, oop y)
{
	oop xArray = asLargeInteger(x)->bytes;
	oop yArray = asLargeInteger(y)->bytes;
	uint32_t xArraySize = totalObjectSize(xArray);
	uint32_t yArraySize = totalObjectSize(yArray);
	uint32_t i;
	
	//If x is longer with non-zero digits, return greater
	if (xArraySize > yArraySize) {
		for (i = xArraySize - 1; i >= yArraySize; i--)
			if (((uint64_t)instVarAtInt(xArray, i)) > 0)
				return 1;
	}
	// if y is longer with non-zero digits return less
	else if (yArraySize > xArraySize) {
		for (i = yArraySize - 1; i >= xArraySize; i--)
			if (((uint64_t)instVarAtInt(yArray, i)) > 0)
				return -1;
	}
	
	// check each value 
	for (i = MIN(xArraySize, yArraySize); i > 0; i--) {
		uint64_t xValue = (uint64_t)instVarAtInt(xArray, i - 1);
		uint64_t yValue = (uint64_t)instVarAtInt(yArray, i - 1);
		if (xValue > yValue)
			return 1;
		if (yValue > xValue)
			return -1;
	}

	// return equal
	return 0;
}

oop allocateLargeInteger(uint32_t size, int sign)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);

	if (sign >= 0) {
		SET_LOCAL(largeIntegerResult, newInstanceOfClass (ST_LARGE_POSITIVE_INTEGER_CLASS, 0, EdenSpace));
	}
	else {
		SET_LOCAL(largeIntegerResult, newInstanceOfClass (ST_LARGE_NEGATIVE_INTEGER_CLASS, 0, EdenSpace));
	}

	SET_LOCAL(largeIntegerArray, newInstanceOfClass (ST_BYTE_ARRAY_CLASS, size * sizeof(oop), EdenSpace));
	instVarAtIntPut(GET_LOCAL(largeIntegerResult), 0, GET_LOCAL(largeIntegerArray));

	asLargeInteger(GET_LOCAL(largeIntegerResult))->componentSize = cIntToST(1);

	oop result = GET_LOCAL(largeIntegerResult);
	FREE_LOCALS;
	return result;
}

oop asSumLargeInteger(int64_t x)
{
	uint64_t topFlags = ((uint64_t) x) & 0xF000000000000000L;

	DEFINE_LOCALS;
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);

	switch (topFlags) {
		case 0x1000000000000000L:
			SET_LOCAL(largeIntegerResult, allocateLargeInteger(1, 1));
			break;
		case 0xE000000000000000L:
			SET_LOCAL(largeIntegerResult, allocateLargeInteger(1, -1));
			break;
        default:
            break;
	}
	
	SET_LOCAL(largeIntegerArray, asLargeInteger(GET_LOCAL(largeIntegerResult))->bytes);
	basicInstVarAtIntPut(GET_LOCAL(largeIntegerArray), 0, x);

	asLargeInteger(GET_LOCAL(largeIntegerResult))->componentSize = cIntToST(computeLargeComponentSize(GET_LOCAL(largeIntegerResult)));
	oop result = GET_LOCAL(largeIntegerResult);
	FREE_LOCALS;

	return result;
}

oop smallToLargeInteger(oop x)
{
	if (isLargeInteger(x))
		return x;

	if (!isSmallInteger(x)) {
		LOGE("Arg not an integer");
		return x;
	}

	int64_t intX = stIntToC(x);
	DEFINE_LOCALS;
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);

	SET_LOCAL(largeIntegerResult, allocateLargeInteger(1, (intX < 0)?-1:1));
	SET_LOCAL(largeIntegerArray, asLargeInteger(GET_LOCAL(largeIntegerResult))->bytes);

	basicInstVarAtIntPut(GET_LOCAL(largeIntegerArray), 0, (uint64_t)(intX < 0)?-intX:intX);
	asLargeInteger(GET_LOCAL(largeIntegerResult))->componentSize = cIntToST(computeLargeComponentSize(GET_LOCAL(largeIntegerResult)));

	oop result = GET_LOCAL(largeIntegerResult);
	FREE_LOCALS;

	return result;
}

void primAsLargeInteger()
{
	oop receiver = getReceiver();	
	oop result = smallToLargeInteger(receiver);
	
	push (cIntToST(0));
	push (result);
}

oop basicLargeIntegerPlus (oop x, oop y, int sign)
{
	uint64_t xComponentSize = largeComponentSize(x);
	uint64_t yComponentSize = largeComponentSize(y);
	uint64_t newComponentSize = MAX(xComponentSize,yComponentSize)+ 1;

	DEFINE_LOCALS;
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	SET_LOCAL(x, x);
	SET_LOCAL(y, y);
	
	SET_LOCAL(largeIntegerResult, allocateLargeInteger((newComponentSize + 1) / 2 + 1, sign));
	SET_LOCAL(largeIntegerArray, asLargeInteger(GET_LOCAL(largeIntegerResult))->bytes);

	uint32_t i, carry = 0;
	
	for (i = 0; i < newComponentSize; i++) {
		uint64_t sum;
		sum = ((uint64_t) largeComponentAt(GET_LOCAL(x), i)) + ((uint64_t) largeComponentAt(GET_LOCAL(y), i)) + carry;
		largeComponentAtPut(GET_LOCAL(largeIntegerResult), i, sum & 0xFFFFFFFFL);
		carry = sum >> 32;
	}

	return GET_LOCAL(largeIntegerResult);
}

oop basicLargeIntegerMinus (oop x, oop y, int sign)
{
	uint64_t xComponentSize = largeComponentSize(x);
	uint64_t yComponentSize = largeComponentSize(y);
	uint64_t newComponentSize = MAX(xComponentSize,yComponentSize)+ 1;

	DEFINE_LOCALS;
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	SET_LOCAL(x, x);
	SET_LOCAL(y, y);
	
	SET_LOCAL(largeIntegerResult, allocateLargeInteger((newComponentSize + 1) / 2 + 1, sign));
	SET_LOCAL(largeIntegerArray, asLargeInteger(GET_LOCAL(largeIntegerResult))->bytes);

	uint32_t i;
	int32_t borrow = 0;
	
	for (i = 0; i < newComponentSize; i++) {
		uint64_t difference;
		difference = ((uint64_t) largeComponentAt(GET_LOCAL(x), i)) - ((uint64_t) largeComponentAt(GET_LOCAL(y), i)) - borrow;
		largeComponentAtPut(GET_LOCAL(largeIntegerResult), i, difference & 0xFFFFFFFFL);
		borrow = (difference & 0xFFFFFFFF00000000L) == 0 ? 0 : 1;
	}

	return GET_LOCAL(largeIntegerResult);
}

void primLargeIntegerPlus()
{
	oop returnValue;

	DEFINE_LOCALS;
	DEFINE_LOCAL(receiver);
	DEFINE_LOCAL(arg);
	DEFINE_LOCAL(result);
	SET_LOCAL(receiver, getReceiver());
	SET_LOCAL(arg, getLocal(0));
	
	if (classOf(GET_LOCAL(receiver)) == classOf(GET_LOCAL(arg))) {
		SET_LOCAL(result, basicLargeIntegerPlus(GET_LOCAL(receiver), GET_LOCAL(arg), classOf(GET_LOCAL(receiver))==ST_LARGE_POSITIVE_INTEGER_CLASS?1:-1));
	}
	else {
		if (largeIntegerABSCompare(GET_LOCAL(receiver), GET_LOCAL(arg)) == 1) {
			SET_LOCAL(result, basicLargeIntegerMinus(GET_LOCAL(receiver), GET_LOCAL(arg), classOf(GET_LOCAL(receiver))==ST_LARGE_POSITIVE_INTEGER_CLASS?1:-1));
		}
		else {
			SET_LOCAL(result, basicLargeIntegerMinus(GET_LOCAL(arg), GET_LOCAL(receiver), classOf(GET_LOCAL(arg))==ST_LARGE_POSITIVE_INTEGER_CLASS?1:-1));
		}
	}
		
	SET_LOCAL(result, largeIntegerReduce(GET_LOCAL(result)));
	returnValue = GET_LOCAL(result);
	FREE_LOCALS;
	push (cIntToST(0));
	push (returnValue);
}

void primLargeIntegerMinus()
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(receiver);
	DEFINE_LOCAL(arg);
	DEFINE_LOCAL(result);
	SET_LOCAL(receiver, getReceiver());
	SET_LOCAL(arg, getLocal(0));

	if (classOf(GET_LOCAL(receiver)) != classOf(GET_LOCAL(arg))) {
		SET_LOCAL(result, basicLargeIntegerPlus(GET_LOCAL(receiver), GET_LOCAL(arg), classOf(GET_LOCAL(receiver))==ST_LARGE_POSITIVE_INTEGER_CLASS?1:-1));
	}
	else {
		if (largeIntegerABSCompare(GET_LOCAL(receiver), GET_LOCAL(arg)) == 1) {
			SET_LOCAL(result, basicLargeIntegerMinus(GET_LOCAL(receiver), GET_LOCAL(arg), classOf(GET_LOCAL(receiver))==ST_LARGE_POSITIVE_INTEGER_CLASS?1:-1));
		}
		else {
			SET_LOCAL(result, basicLargeIntegerMinus(GET_LOCAL(arg), GET_LOCAL(receiver), classOf(GET_LOCAL(receiver))==ST_LARGE_POSITIVE_INTEGER_CLASS?-1:1));
		}
	}

	SET_LOCAL(result, largeIntegerReduce(GET_LOCAL(result)));
	push (cIntToST(0));
	push (GET_LOCAL(result));
}

void primLargeIntegerLessThan()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == -1)?ST_TRUE:ST_FALSE);
}

void primLargeIntegerGreaterThan()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == 1)?ST_TRUE:ST_FALSE);
}

void primLargeIntegerLessThanOrEqual()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == 1)?ST_FALSE:ST_TRUE);
}

void primLargeIntegerGreaterThanOrEqual()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == -1)?ST_FALSE:ST_TRUE);
}

void primLargeIntegerEqual()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_FALSE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == 0)?ST_TRUE:ST_FALSE);
}

void primLargeIntegerNotEqual()
{
	oop receiver = getReceiver();
	oop arg = getLocal(0);

	if (isLargePositiveInteger(receiver) && isLargeNegativeInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	if (isLargeNegativeInteger(receiver) && isLargePositiveInteger(arg)) {
		push (cIntToST(0));
		push (ST_TRUE);
		return;
	}

	push (cIntToST(0));
	push ((largeIntegerABSCompare(receiver, arg) == 0)?ST_FALSE:ST_TRUE);
}

oop extractComponents(oop intOop, uint32_t start, uint32_t end)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(source);
	DEFINE_LOCAL(result);
	SET_LOCAL(source, intOop);
	// allocate a little extra space for the inplace add.
	oop result = allocateLargeInteger((end-start)/2+2, 1);
	SET_LOCAL(result, result);

	uint32_t i;
	for (i=0; i <= end-start; i++) {
		uint32_t component = largeComponentAt(GET_LOCAL(source), start + i);
		largeComponentAtPut(GET_LOCAL(result), i, component);
	}

	result = GET_LOCAL(result);
	FREE_LOCALS;

	return result;
}

oop combineProduct(oop ac, oop ad_plus_bc, oop bd, uint32_t size)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(ac);
	DEFINE_LOCAL(ad_plus_bc);
	DEFINE_LOCAL(bd);
	SET_LOCAL(ac, ac);
	SET_LOCAL(ad_plus_bc, ad_plus_bc);
	SET_LOCAL(bd, bd);
	DEFINE_LOCAL(result);

	oop result = allocateLargeInteger(size * 4, 1);
	uint64_t component = 0;

	SET_LOCAL(result, result);

	uint32_t i;
	for (i=0; i<size; i++) {
		component += largeComponentAt(GET_LOCAL(bd), i);
		largeComponentAtPut(GET_LOCAL(result), i, component & 0xFFFFFFFFL);
		component >>= 32;
		
	}
	
	for (i=0; i<size; i++) {
		component += largeComponentAt(GET_LOCAL(bd), size + i);
		component += largeComponentAt(GET_LOCAL(ad_plus_bc), i);
		largeComponentAtPut(GET_LOCAL(result), size+i, component & 0xFFFFFFFFL);
		component >>= 32;
	}

	for (i=0; i<size; i++) {
		component += largeComponentAt(GET_LOCAL(ad_plus_bc), size + i);
		component += largeComponentAt(GET_LOCAL(ac), i);
		largeComponentAtPut(GET_LOCAL(result), (size*2) + i, component & 0xFFFFFFFFL);
		component >>= 32;
	}

	for (i=0; i<size; i++) {
		component += largeComponentAt(GET_LOCAL(ac), size + i);
		largeComponentAtPut(GET_LOCAL(result), (size*3) + i, component & 0xFFFFFFFFL);
		component >>= 32;
	}

	result = GET_LOCAL(result);
	FREE_LOCALS;
	return result;
}

void inplacePlus(oop x, oop y)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	SET_LOCAL(x, x);
	SET_LOCAL(y, y);

	uint64_t xComponentSize = largeComponentSize(x);
	uint64_t yComponentSize = largeComponentSize(y);
	uint64_t newComponentSize = MAX(xComponentSize,yComponentSize)+ 1;

	uint32_t i;
	uint32_t carry = 0;
	
	for (i = 0; i < newComponentSize + 1; i++) {
		uint64_t sum;
		sum = ((uint64_t) largeComponentAt(GET_LOCAL(x), i)) + ((uint64_t) largeComponentAt(GET_LOCAL(y), i)) + carry;
		largeComponentAtPut(GET_LOCAL(x), i, sum & 0xFFFFFFFFL);
		carry = sum >> 32;
	}
	FREE_LOCALS;
}

void inplaceMinus(oop x, oop y)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	SET_LOCAL(x, x);
	SET_LOCAL(y, y);

	uint64_t xComponentSize = largeComponentSize(x);
	uint64_t yComponentSize = largeComponentSize(y);
	uint64_t newComponentSize = MAX(xComponentSize,yComponentSize)+ 1;

	uint32_t i;
	int32_t borrow = 0;
	
	for (i = 0; i < newComponentSize; i++) {
		uint64_t difference;
		difference = ((uint64_t) largeComponentAt(GET_LOCAL(x), i)) - ((uint64_t) largeComponentAt(GET_LOCAL(y), i)) - borrow;
		largeComponentAtPut(GET_LOCAL(x), i, difference & 0xFFFFFFFFL);
		borrow = (difference & 0xFFFFFFFF00000000L) == 0 ? 0 : 1;
	}
	FREE_LOCALS;
}

oop karatsuba(oop x, oop y, int sign)
{
	if (isZero(x)) {
		return allocateLargeInteger(totalObjectSize(x), sign);
	}

	if (isZero(y)) {
		return allocateLargeInteger(totalObjectSize(x), sign);
	}

	DEFINE_LOCALS;
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	
	SET_LOCAL(x,x);
	SET_LOCAL(y,y);
	
	
	uint32_t xSize = largeComponentSize(x);
	uint32_t ySize = largeComponentSize(y);
	uint32_t maxSize = MAX(xSize, ySize);
	
	if (maxSize == 1) {
		uint64_t xComponent = largeComponentAt(GET_LOCAL(x), 0);
		uint64_t yComponent = largeComponentAt(GET_LOCAL(y), 0);
		if (xComponent == 0) {
			oop result = allocateLargeInteger(1, sign);
			FREE_LOCALS;
			return result;
		}
		if (yComponent == 0) {
			oop result = allocateLargeInteger(1, sign);
			FREE_LOCALS;
			return result;
		}
		uint64_t product =  xComponent * yComponent;
		oop result = allocateLargeInteger(1, sign);
		largeComponentAtPut(result, 0, product & 0xFFFFFFFFL);
		largeComponentAtPut(result, 1, product >> 32);
		FREE_LOCALS;
		return result;
	}
	
	DEFINE_LOCAL(a);
	DEFINE_LOCAL(b);
	DEFINE_LOCAL(c);
	DEFINE_LOCAL(d);
	
	DEFINE_LOCAL(ac);
	DEFINE_LOCAL(bd);

	DEFINE_LOCAL(ad_plus_bc);
	
	uint32_t subSize = (maxSize + 1) / 2;
	oop tempA = extractComponents(GET_LOCAL(x), subSize, maxSize-1);
	SET_LOCAL(a, tempA);
	oop tempB = extractComponents(GET_LOCAL(x), 0, subSize-1);
	SET_LOCAL(b, tempB);
	oop tempC = extractComponents(GET_LOCAL(y), subSize, maxSize-1);
	SET_LOCAL(c, tempC);
	oop tempD = extractComponents(GET_LOCAL(y), 0, subSize-1);
	SET_LOCAL(d, tempD);

	oop tempAC = karatsuba(GET_LOCAL(a), GET_LOCAL(c), sign);
	SET_LOCAL(ac, tempAC);

	oop tempBD = karatsuba(GET_LOCAL(b), GET_LOCAL(d), sign);
	SET_LOCAL(bd, tempBD);

	inplacePlus(GET_LOCAL(a), GET_LOCAL(b));
	inplacePlus(GET_LOCAL(c), GET_LOCAL(d));
	
	oop ad_plus_bc = karatsuba(GET_LOCAL(a), GET_LOCAL(c), sign);
	SET_LOCAL(ad_plus_bc, ad_plus_bc);
	inplaceMinus(GET_LOCAL(ad_plus_bc), GET_LOCAL(ac));
	inplaceMinus(GET_LOCAL(ad_plus_bc), GET_LOCAL(bd));
	
	oop result = combineProduct(GET_LOCAL(ac), GET_LOCAL(ad_plus_bc), GET_LOCAL(bd), subSize);

	FREE_LOCALS;
	return result;
}

oop largeIntegerTimes(oop x, oop y)
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(x);
	DEFINE_LOCAL(y);
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);

	SET_LOCAL(x,x);
	SET_LOCAL(y,y);

	int sign;

	if (classOf(GET_LOCAL(x)) == classOf(GET_LOCAL(y)))
		sign = 1;
	else
		sign = -1;

	uint64_t xSize = largeComponentSize(GET_LOCAL(x));
	uint64_t ySize = largeComponentSize(GET_LOCAL(y));
	uint64_t newComponentSize = xSize + ySize;
	
	SET_LOCAL(largeIntegerResult, allocateLargeInteger((newComponentSize + 1) / 2, sign));

	uint64_t totalComponents = MAX(xSize, ySize);
	uint64_t i, j;

	oop x1 = GET_LOCAL(x);
	oop y1 = GET_LOCAL(y);
	oop result = GET_LOCAL(largeIntegerResult);
	FREE_LOCALS;

	// the rest of the method doesn't need to allocate space so we can use temp vars instead of locals for speed
	for (i = 0; i < xSize; i++)
		for (j = 0; j < ySize; j++) {
			uint64_t product = largeComponentAt(x1, i) * largeComponentAt(y1,j);
			if (product != 0)
				largeComponentAtAdd(result, i + j, product);
		}

	result = largeIntegerReduce(result);
    return result;
}

void primLargeIntegerTimes()
{
	if ((!isLargeInteger(getReceiver())) || (!isLargeInteger(getLocal(0)))) {
		push (cIntToST(1));
		push (getReceiver());
		return;
	}
	
	oop result = largeIntegerTimes(getReceiver(), getLocal(0));
	push (cIntToST(0));
	push (result);
}

void primLargeIntegerTimesFast()
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(receiver);
	DEFINE_LOCAL(arg);
	DEFINE_LOCAL(largeIntegerResult);
	DEFINE_LOCAL(largeIntegerArray);

	SET_LOCAL(receiver, getReceiver());
	SET_LOCAL(arg, getLocal(0));

	int sign;

	if ((!isLargeInteger(GET_LOCAL(receiver))) || (!isLargeInteger(GET_LOCAL(arg)))) {
		FREE_LOCALS;
		push (cIntToST(1));
		push (cIntToST(1));
		return;
	}
	
	if (classOf(GET_LOCAL(receiver)) == classOf(GET_LOCAL(arg)))
		sign = 1;
	else
		sign = -1;

	uint64_t receiverSize = largeComponentSize(GET_LOCAL(receiver));
	uint64_t argSize = largeComponentSize(GET_LOCAL(arg));
	uint64_t newComponentSize = receiverSize + argSize;
	
	oop result = karatsuba(GET_LOCAL(receiver), GET_LOCAL(arg), sign);
	SET_LOCAL(largeIntegerResult, result);
	result = largeIntegerReduce(GET_LOCAL(largeIntegerResult));

	FREE_LOCALS;
	push (cIntToST(0));
	push (result);
}

void copyComponentsFromToSize(oop source, oop dest, uint64_t numberOfComponents)
{
	int componentNumber;
	uint64_t component;

	for (componentNumber = largeComponentSize(source) - 1; numberOfComponents-- > 0; componentNumber--) {
		component = largeComponentAt(source, componentNumber);
		largeComponentAtPut(dest, numberOfComponents, component);
	}
}

void largeIntegerShiftComponents(oop number, uint64_t shiftNumber)
{
	int32_t componentSize = largeComponentSize(number);
	int32_t i;
	for (i = componentSize - 1; i >= 0; i--)
		largeComponentAtPut(number, i+1, largeComponentAt(number, i));
}

oop largeTimesFromToInt(oop source, oop dest, uint64_t number2)
{
	int32_t i;
	uint64_t sourceSize = largeComponentSize(source), product = 0;
	oop destIntegerArray = asLargeInteger(dest)->bytes;

	// Clear the destination to all zeros
	for (i=0; i<totalObjectSize(destIntegerArray); i++) {
		basicInstVarAtIntPut(destIntegerArray, i, 0);
	}
		
	for (i=0; i < sourceSize; i++) {
		uint64_t component = largeComponentAt(source,i);
		product = component * number2 + product;
		largeComponentAtPut(dest, i, product & 0xFFFFFFFFL);
		product = product >> 32;
	}
	
	if (product > 0)
		largeComponentAtPut(dest, sourceSize, product & 0xFFFFFFFFL);
	
	return dest;
}

int largeIntegerDivideWithRemainder (oop dividend, oop divisor, oop quotient, oop remainder)
{
	uint64_t dividendSize = largeComponentSize(dividend);
	uint64_t divisorSize = largeComponentSize(divisor);
	uint64_t divisorComponent, dividendTwoComponents, quotientDigit, scalingFactor;
	int64_t quotientDigitNumber;

	if (isZero(divisor)) {
		return FALSE;
	}

	DEFINE_LOCALS;
	DEFINE_LOCAL(dividend);
	DEFINE_LOCAL(divisor);
	DEFINE_LOCAL(quotient);
	DEFINE_LOCAL(remainder);
	DEFINE_LOCAL(intermediateDividend);
	DEFINE_LOCAL(newDividend);
	DEFINE_LOCAL(normalizedDividend);
	DEFINE_LOCAL(normalizedDivisor);

	SET_LOCAL(dividend, dividend);
	SET_LOCAL(divisor, divisor);
	SET_LOCAL(quotient, quotient);
	SET_LOCAL(remainder, remainder);
	SET_LOCAL(intermediateDividend, allocateLargeInteger((dividendSize + 1) / 2 + 1, 1));

	divisorComponent = largeComponentAt(GET_LOCAL(divisor), divisorSize - 1);
	if ((divisorSize > 1) && divisorComponent < 0x80000000L) {
		SET_LOCAL(normalizedDividend, allocateLargeInteger((dividendSize + 1) / 2 + 2, 1));
		SET_LOCAL(normalizedDivisor, allocateLargeInteger((divisorSize + 1) / 2 + 2, 1));

		scalingFactor = 0x100000000L / (divisorComponent + 1);
		copyComponentsFromToSize(GET_LOCAL(dividend), GET_LOCAL(normalizedDividend), dividendSize);
		copyComponentsFromToSize(GET_LOCAL(divisor), GET_LOCAL(normalizedDivisor), divisorSize);
		largeTimesFromToInt(GET_LOCAL(dividend), GET_LOCAL(normalizedDividend), scalingFactor);
		largeTimesFromToInt(GET_LOCAL(divisor), GET_LOCAL(normalizedDivisor), scalingFactor);
		
		divisorSize = largeComponentSize(GET_LOCAL(normalizedDivisor));
		dividendSize = largeComponentSize(GET_LOCAL(normalizedDividend));
		divisorComponent = largeComponentAt(GET_LOCAL(normalizedDivisor), divisorSize - 1);
	} else {
		scalingFactor = 1;
		SET_LOCAL(normalizedDividend, GET_LOCAL(dividend));
		SET_LOCAL(normalizedDivisor, GET_LOCAL(divisor));
	}

	SET_LOCAL(newDividend, allocateLargeInteger((divisorSize + 1) / 2 + 1, 1));
	copyComponentsFromToSize(GET_LOCAL(normalizedDividend), GET_LOCAL(intermediateDividend), divisorSize);
	quotientDigitNumber = (int64_t)(dividendSize - divisorSize);
	
	while (quotientDigitNumber >= 0) {
		if (largeIntegerABSCompare(GET_LOCAL(intermediateDividend), GET_LOCAL(normalizedDivisor)) == -1) {
			quotientDigit = 0;
		} else {
			dividendTwoComponents = (largeComponentAt(GET_LOCAL(intermediateDividend), divisorSize) << 32) | largeComponentAt(GET_LOCAL(intermediateDividend), divisorSize - 1);
			quotientDigit = MIN(dividendTwoComponents / divisorComponent, 0xFFFFFFFF);

			largeTimesFromToInt(GET_LOCAL(normalizedDivisor), GET_LOCAL(newDividend), quotientDigit);
			while (largeIntegerABSCompare(GET_LOCAL(newDividend), GET_LOCAL(intermediateDividend))==1) {
				quotientDigit = quotientDigit - 1;
				inplaceMinus(GET_LOCAL(newDividend), GET_LOCAL(normalizedDivisor));
			}

			inplaceMinus(GET_LOCAL(intermediateDividend), GET_LOCAL(newDividend));
		}

		if (quotientDigitNumber > 0) {
			uint64_t pullDown = largeComponentAt(GET_LOCAL(normalizedDividend), quotientDigitNumber - 1);
			largeIntegerShiftComponents(GET_LOCAL(intermediateDividend), 1);
			largeComponentAtPut(GET_LOCAL(intermediateDividend), 0, pullDown);
		}
		
		if (quotientDigit > 0) {
			largeComponentAtPut(GET_LOCAL(quotient), quotientDigitNumber, quotientDigit);
		}
		
		quotientDigitNumber--;
	}

	if (scalingFactor == 1) {
		copyComponentsFromToSize(GET_LOCAL(intermediateDividend), GET_LOCAL(remainder), largeComponentSize(GET_LOCAL(intermediateDividend)));
	} else {
		DEFINE_LOCAL(scalingFactor);
		DEFINE_LOCAL (scalingRemainder);
		SET_LOCAL(scalingFactor, allocateLargeInteger(1, 1));
		SET_LOCAL(scalingRemainder, allocateLargeInteger(1, 1));

		basicInstVarAtIntPut(instVarAtInt(GET_LOCAL(scalingFactor), 0), 0, scalingFactor);
		largeIntegerDivideWithRemainder(GET_LOCAL(intermediateDividend), GET_LOCAL(scalingFactor), GET_LOCAL(remainder), GET_LOCAL(scalingRemainder));
	}

	FREE_LOCALS;
	return TRUE;
}

void primLargeIntegerDivide()
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(dividend);
	DEFINE_LOCAL(divisor);
	DEFINE_LOCAL(quotient);
	DEFINE_LOCAL(remainder);

	SET_LOCAL(dividend, getReceiver());
	SET_LOCAL(divisor, getLocal(0));
	int sign;

	if ((!isLargeInteger(GET_LOCAL(dividend))) || (!isLargeInteger(GET_LOCAL(divisor)))) {
		FREE_LOCALS;
		push (cIntToST(1));
		push (GET_LOCAL(dividend));
		return;
	}

	if (classOf(GET_LOCAL(dividend)) == classOf(GET_LOCAL(divisor)))
		sign = 1;
	else
		sign = -1;

	uint64_t dividendSize = largeComponentSize(GET_LOCAL(dividend));
	uint64_t divisorSize = largeComponentSize(GET_LOCAL(divisor));
	uint64_t quotientSize = dividendSize - divisorSize + 1;
	
	SET_LOCAL(quotient, allocateLargeInteger((quotientSize + 1) / 2, sign));
	SET_LOCAL(remainder, allocateLargeInteger((divisorSize + 1) / 2, 1));

	if (!largeIntegerDivideWithRemainder (GET_LOCAL(dividend), GET_LOCAL(divisor), GET_LOCAL(quotient), GET_LOCAL(remainder))) {
		FREE_LOCALS;
		push(cIntToST(1));
		push(cIntToST(1));
		return;
	}
	
	oop result = largeIntegerReduce(GET_LOCAL(quotient));
	FREE_LOCALS;
	
	push (cIntToST(0));
	push (result);
}

void primLargeIntegerModulo()
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(dividend);
	DEFINE_LOCAL(divisor);
	DEFINE_LOCAL(quotient);
	DEFINE_LOCAL(remainder);

	SET_LOCAL(dividend, getReceiver());
	SET_LOCAL(divisor, getLocal(0));
	oop sign;

	if ((!isLargeInteger(GET_LOCAL(dividend))) || (!isLargeInteger(GET_LOCAL(divisor)))) {
		push (cIntToST(1));
		push (GET_LOCAL(dividend));
		return;
	}

	if (classOf(GET_LOCAL(dividend)) == classOf(GET_LOCAL(divisor)))
		sign = 1;
	else
		sign = -1;

	uint64_t dividendSize = largeComponentSize(GET_LOCAL(dividend));
	uint64_t divisorSize = largeComponentSize(GET_LOCAL(divisor));
	uint64_t quotientSize = dividendSize - divisorSize + 1;
	
	SET_LOCAL(quotient, allocateLargeInteger((quotientSize + 1) / 2, sign));
	SET_LOCAL(remainder, allocateLargeInteger((divisorSize + 1) / 2, 1));

	if (!largeIntegerDivideWithRemainder (GET_LOCAL(dividend), GET_LOCAL(divisor), GET_LOCAL(quotient), GET_LOCAL(remainder))) {
		push(cIntToST(1));
		push(cIntToST(1));
		return;
	}
	
	oop result = largeIntegerReduce(GET_LOCAL(remainder));
	FREE_LOCALS;

	push (cIntToST(0));
	push (result);
}

void primLargeIntegerDivideWithRemainder()
{
	DEFINE_LOCALS;
	DEFINE_LOCAL(dividend);
	DEFINE_LOCAL(divisor);
	DEFINE_LOCAL(quotient);
	DEFINE_LOCAL(remainder);

	SET_LOCAL(dividend, getReceiver());
	SET_LOCAL(divisor, getLocal(0));
	oop sign;

	if ((!isLargeInteger(GET_LOCAL(dividend))) || (!isLargeInteger(GET_LOCAL(divisor)))) {
		FREE_LOCALS;
		push (cIntToST(1));
		push (cIntToST(1));
		return;
	}

	if (classOf(GET_LOCAL(dividend)) == classOf(GET_LOCAL(divisor)))
		sign = 1;
	else
		sign = -1;

	uint64_t dividendSize = largeComponentSize(GET_LOCAL(dividend));
	uint64_t divisorSize = largeComponentSize(GET_LOCAL(divisor));
	uint64_t quotientSize = dividendSize - divisorSize + 1;
	
	SET_LOCAL(quotient, allocateLargeInteger((quotientSize + 1) / 2, sign));
	SET_LOCAL(remainder, allocateLargeInteger((divisorSize + 1) / 2, 1));

	if (!largeIntegerDivideWithRemainder (GET_LOCAL(dividend), GET_LOCAL(divisor), GET_LOCAL(quotient), GET_LOCAL(remainder))) {
		FREE_LOCALS;
		push(cIntToST(1));
		push(cIntToST(2));
		return;
	}
	
	DEFINE_LOCAL(result);
	SET_LOCAL(result, newInstanceOfClass(ST_ARRAY_CLASS, 2, EdenSpace));
	
	oop localQuotient = largeIntegerReduce(GET_LOCAL(quotient));
	instVarAtIntPut(GET_LOCAL(result), 0, localQuotient);

	oop localRemainder = largeIntegerReduce(GET_LOCAL(remainder));
	instVarAtIntPut(GET_LOCAL(result), 1, localRemainder);

	oop result = GET_LOCAL(result);
	FREE_LOCALS;

	push (cIntToST(0));
	push (result);
}

void primLargeIntegerAsFloat()
{
	oop receiver = getReceiver();
	if (!isLargeInteger(receiver)) {
		push (cIntToST(1));
		push (cIntToST(1));
		return;
	}

	double result = 0.0;
	int32_t i;

	for (i = largeComponentSize(receiver)-1; i >= 0; i--) {
		result = result * ((double)0x100000000L) + ((double)largeComponentAt(receiver, i));
	}

	oop resultOop = cFloatToST(result);

	push (cIntToST(0));
	push (resultOop);
}

void primLargeIntegerIntDivide()
{
	push (cIntToST(0));
	push (cIntToST(1));
}

void primBitAnd(){
	uint64_t receiver = stIntToC(getReceiver());
	uint64_t mask = stIntToC(getLocal( 0));

	push (cIntToST(0));
	push (cIntToST(receiver & mask));
}
void primBitOr(){
	uint64_t receiver = stIntToC(getReceiver());
	uint64_t mask = stIntToC(getLocal( 0));

	push (cIntToST(0));
	push (cIntToST(receiver | mask));
}
void primBitXor(){
	uint64_t receiver = stIntToC(getReceiver());
	uint64_t mask = stIntToC(getLocal( 0));

	push (cIntToST(0));
	push (cIntToST(receiver ^ mask));
}
void primBitInvert(){
	uint64_t receiver = stIntToC(getReceiver());

	push (cIntToST(0));
	push (cIntToST(~receiver));
}


void initializeIntegerPrimitives()
{
	primitiveTable[PRIM_SMALLINTEGER_PLUS] = primSmallIntegerPlus;
	primitiveTable[PRIM_SMALLINTEGER_MINUS] = primSmallIntegerMinus;
	primitiveTable[PRIM_SMALLINTEGER_LESS_THAN] = primSmallIntegerLessThan;
	primitiveTable[PRIM_SMALLINTEGER_GREATER_THAN] = primSmallIntegerGreaterThan;
	primitiveTable[PRIM_SMALLINTEGER_LESS_THAN_OR_EQUAL] = primSmallIntegerLessThanOrEqual;
	primitiveTable[PRIM_SMALLINTEGER_GREATER_THAN_OR_EQUAL] = primSmallIntegerGreaterThanOrEqual;
	primitiveTable[PRIM_SMALLINTEGER_EQUAL] = primSmallIntegerEqual;
	primitiveTable[PRIM_SMALLINTEGER_NOT_EQUAL] = primSmallIntegerNotEqual;
	primitiveTable[PRIM_SMALLINTEGER_TIMES] = primSmallIntegerTimes;
	primitiveTable[PRIM_SMALLINTEGER_DIVIDE] = primSmallIntegerDivide;
	primitiveTable[PRIM_SMALLINTEGER_MODULO] = primSmallIntegerModulo;
	primitiveTable[PRIM_SMALLINTEGER_INT_DIVIDE] = primSmallIntegerIntDivide;
	primitiveTable[PRIM_SMALLINTEGER_AS_FLOAT] = primSmallIntegerAsFloat;
	primitiveTable[PRIM_BITSHIFT] = primBitShift;

	primitiveTable[PRIM_AS_LARGEINTEGER] = primAsLargeInteger;
	primitiveTable[PRIM_LARGEINTEGER_PLUS] = primLargeIntegerPlus;
	primitiveTable[PRIM_LARGEINTEGER_MINUS] = primLargeIntegerMinus;
	primitiveTable[PRIM_LARGEINTEGER_LESS_THAN] = primLargeIntegerLessThan;
	primitiveTable[PRIM_LARGEINTEGER_GREATER_THAN] = primLargeIntegerGreaterThan;
	primitiveTable[PRIM_LARGEINTEGER_LESS_THAN_OR_EQUAL] = primLargeIntegerLessThanOrEqual;
	primitiveTable[PRIM_LARGEINTEGER_GREATER_THAN_OR_EQUAL] = primLargeIntegerGreaterThanOrEqual;
	primitiveTable[PRIM_LARGEINTEGER_EQUAL] = primLargeIntegerEqual;
	primitiveTable[PRIM_LARGEINTEGER_NOT_EQUAL] = primLargeIntegerNotEqual;
//	primitiveTable[PRIM_LARGEINTEGER_TIMES] = primLargeIntegerTimesFast;
	primitiveTable[PRIM_LARGEINTEGER_TIMES] = primLargeIntegerTimes;
	primitiveTable[PRIM_LARGEINTEGER_DIVIDE] = primLargeIntegerDivide;
	primitiveTable[PRIM_LARGEINTEGER_MODULO] = primLargeIntegerModulo;
	primitiveTable[PRIM_LARGEINTEGER_INT_DIVIDE] = primLargeIntegerIntDivide;
	primitiveTable[PRIM_LARGEINTEGER_DIVIDE_WITH_REMAINDER] = primLargeIntegerDivideWithRemainder;
	primitiveTable[PRIM_LARGEINTEGER_TIMES_FAST] = primLargeIntegerTimesFast;
	primitiveTable[PRIM_LARGEINTEGER_AS_FLOAT] = primLargeIntegerAsFloat;

	primitiveTable[PRIM_BIT_AND] = primBitAnd;
	primitiveTable[PRIM_BIT_OR] = primBitOr;
	primitiveTable[PRIM_BIT_XOR] = primBitXor;
	primitiveTable[PRIM_BIT_INVERT] = primBitInvert;
}
