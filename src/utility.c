// utility.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

//
// Created by maxim on 2017-06-02.
/* This file provides utility extern functions for use alongside the many macros of object.h to make VM logic more readable and intuitive for the end user in smalltalk.)*/
//

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "object.h"

// Takes a smalltalk (string) and write's its data into a null terminated string (cstring) for use in C.
int STStringToC(oop string, char *cstring){
    int sizeOfString = (int) basicByteSize(string);
    for(int i = 0; i < sizeOfString; i++) cstring[i] = basicByteAtInt(string, i + 1);
		cstring[sizeOfString] = '\0';

    return 0;
}

// Takes a C UTF string and writes it's data into a smalltalk object string
oop CStringToST(char *cstring){
	int sizeOfString = strlen(cstring);
	oop stStringOop = newInstanceOfClass(ST_BYTE_STRING_CLASS, sizeOfString, EdenSpace);
 
    for(int i = 0; i < sizeOfString; i++)
		basicByteAtIntPut(stStringOop, i + 1, cstring[i]);

    return stStringOop;
}

// Takes a smalltalk collection of strings and writes each string's data into a null terminated string who's reference is inserted into an array of pointers.
int stStringCollectionToC(oop stringCollection, char ***cArray){
    int sizeOfCollection = (int) indexedObjectSize(stringCollection);
    LOGW("Creating an array of %d fields", sizeOfCollection);
    *cArray = malloc(sizeof(char*) * sizeOfCollection);
    LOGW("Allocated %lu bytes of memory for a cstring array", (unsigned long) sizeof(char*) * sizeOfCollection);
    for(int i = 1; i <= sizeOfCollection; i++){
        oop stStringOop = indexedVarAtInt(stringCollection, i);
        char *cString = malloc((unsigned long) basicByteSize(stStringOop) + 1);
        STStringToC(stStringOop, cString);
        LOGW("Generated cString for: '%s'", cString);
        (*cArray)[i-1] = cString;
    }

    for(int i = 0; i < sizeOfCollection; ++i){
        LOGW("%s is included in the array at position %d", (*cArray)[i], i);
    }

    return 0;
}

// frees an array of pointers to dynamically allocated strings
void freeStringCollection(char **cArray){
    for(int i = 0; i < sizeof(cArray)/sizeof(char*); i++){
        free(cArray[i]);
    }

    free(cArray);

return;
}

oop identityDictionaryAt (oop dictionary, oop key)
{
    oop identityDictionaryOop = dictionary;
	oop arrayOop = asIdentityDictionary(identityDictionaryOop)->values;
	oop assocOop;

	uint64_t dictionarySize = indexedObjectSize(arrayOop);
	if (dictionarySize == 0)
		return (ST_NIL);

	uint64_t index = asObjectHeader(key)->identityHash % dictionarySize;
	uint64_t startingIndex = index;
    
	while ((assocOop = instVarAtInt(arrayOop,index)) != ST_NIL) {
		if (asAssociation(assocOop)->key == key) {
			return asAssociation(assocOop)->value;
		}

		index++;
		if (index >= dictionarySize) {
			index = 0;
		}

		if (index == startingIndex)
			break;
	}

	return ST_NIL;
}

oop identityDictionaryKeyAtValue (oop dictionary, oop value)
{
	oop arrayOop = asIdentityDictionary(dictionary)->values;

	uint64_t dictionarySize = indexedObjectSize(arrayOop);
    int i;

	for (i=0; i<dictionarySize; i++) {
		oop assocOop = instVarAtInt(arrayOop, i);
		if (assocOop != ST_NIL)
			if (asAssociation(assocOop)->value == value)
				return (asAssociation(assocOop)->key);
	}
		
	return ST_NIL;
}

oop stPtrToC(oop x)
{
	if ((x)==asOop(NULL))
		return asOop(NULL);

	if (isImmediate(x))
		return (x);

	unsigned int spaceNumber;

	spaceNumber = ((x >> 48) - 1) & 0xFF;

	int offset = ((x) & 0xFFFFFFFFFFFFF8) >> IMMEDIATE_SHIFT;
	memorySpaceStruct *space = Spaces[spaceNumber];
	return asOop(&((uint8_t *)space->space) [offset]);
}

int32_t spaceNumberOfOop(oop p)
{
	int32_t i;

	for (i=0; i<MAX_SPACES; i++)
	{
		if (Spaces[i]->spaceSize == 0)
			return 0;

		if ((p >= asOop(&Spaces[i]->space[0])) && (p <= asOop(&Spaces[i]->space[Spaces[i]->spaceSize / sizeof(oop)]))) {
			return i;
		}
	}
	return -1;
}

uint64_t oopToOffset (oop p)
{
	int32_t spaceNumber;

	if (isImmediate(p))
		return (uint64_t) p;

	if (((uint64_t) p) == 0)
		return (uint64_t) p;

	spaceNumber = (uint64_t)spaceNumberOfOop(p);
	if (spaceNumber < 0) {
		LOGE ("oopToOffset: bad space for pointer %"PRIx64, p);
		return 0;
	}
	
	uint64_t offset = (p - asOop(&Spaces[spaceNumber]->space[0]));
	uint64_t result;
	
	result = ((((uint64_t)spaceNumber+1) << 48) | (offset << 3));

	return result;
}

oop globalVariableAt(oop symbol)
{
	return identityDictionaryAt (ST_SYSTEM_DICTIONARY, symbol);
}

// Posts a constructed event directly to the event queue, allowing for multiple events to be posted by the VM such that they all get interpreted
void postConstructedEventToQueue(oop event){
}

void raiseSTError(oop errorClass, char *message)
{
	push (errorClass);
	oop stMessage = CStringToST(message);
	oop recoveredErrorClass = pop ();
	push (stMessage);
	oop stException = newInstanceOfClass(recoveredErrorClass, 0, EdenSpace);
	oop recoveredMessage = pop ();
	asException(stException)->message = recoveredMessage;
	registerIfNeeded(stException, recoveredMessage);
	push (stException);
	dispatchSpecial0(SPECIAL_RAISE_SIGNAL, stException);
}

oop contextCopy(oop contextOop)
{

	oop newFrame, oldFrame;

	DEFINE_LOCALS;
	DEFINE_LOCAL (parentContext);
	DEFINE_LOCAL (frameOop);
	DEFINE_LOCAL (topFrame);
	SET_LOCAL (parentContext, ST_NIL);

	oldFrame = contextOop;
	
	while (asContext(oldFrame)->method != ST_NIL) {
		int64_t stackIndex;
		
		SET_LOCAL(frameOop, oldFrame);
		newFrame = newInstanceOfClass(ST_CODE_CONTEXT_CLASS, indexedObjectSize(oldFrame), EdenSpace);
		oldFrame = GET_LOCAL(frameOop);
	
		asContext(newFrame)->stackOffset = asContext(oldFrame)->stackOffset;
		asContext(newFrame)->pcOffset = asContext(oldFrame)->pcOffset;
		asContext(newFrame)->method = asContext(oldFrame)->method;
		asContext(newFrame)->methodContext = asContext(oldFrame)->methodContext;
		asContext(newFrame)->contextId = markAsSmallInteger(stripTags(asContext(oldFrame)->contextId));

		asContext(newFrame)->frame = ST_NIL;
	
		for (stackIndex = 0; stackIndex < indexedObjectSize(oldFrame); stackIndex++) {
			asContext(newFrame)->stackBody[stackIndex] = asContext(oldFrame)->stackBody[stackIndex];
		}

		if (GET_LOCAL(parentContext) != ST_NIL) {
			asContext(GET_LOCAL(parentContext))->frame = newFrame;
		}
		else {
			SET_LOCAL (topFrame, newFrame);
		}
	
		SET_LOCAL (parentContext, newFrame);
		oldFrame = asContext(oldFrame)->frame;
	}

	newFrame = GET_LOCAL(topFrame);
	FREE_LOCALS;

	return newFrame;

//	return contextOop;
}
