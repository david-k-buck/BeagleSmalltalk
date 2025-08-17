// image.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "object.h"

unsigned long Development = 1;
char ImageName[256];

void relocateObject (oop object, __attribute__((unused)) void *args)
{
	if ((asObjectHeader(object)->flags & FREE) == FREE)
		return;

	asObjectHeader(object)->stClass = stPtrToC(asObjectHeader(object)->stClass);
	asObjectHeader(object)->bodyPointer = stPtrToC(asObjectHeader(object)->bodyPointer);

	setBodyHeaderPointer(object);
	if (isBytes(object))
		return;

	uint64_t i;

	for (i=0; i<totalObjectSize(object); i++) {
		oop relocatedObject = stPtrToC(instVarAtInt(object, i));
		instVarAtIntPut(object, i, relocatedObject);
	}
}

void relocateObjectSpace (memorySpaceStruct *space)
{
	enumerateObjectsInSpace (space, relocateObject, NULL);
}

void relocatePointer (oop *oopPointer, __attribute__((unused)) void *args)
{
	if (*oopPointer != asOop(NULL)) {
        if (!isImmediate(*oopPointer))
		    *oopPointer = stPtrToC(*oopPointer);
	}
}

void relocatePointerSpace (memorySpaceStruct *space)
{
	enumeratePointersInSpace (space, relocatePointer, NULL);	
}

void relocateStackSpace (__attribute__((unused)) memorySpaceStruct *space)
{
}

uint64_t readSpace(memorySpaceStruct **allocatedSpacePtr, readFunctionType *readFunction, void *data)
{
	memorySpaceStruct memorySpace, *allocatedSpace;
	readFunction((unsigned char *) &memorySpace, sizeof(memorySpace), data);

	allocatedSpace = allocateSpace (memorySpace.spaceSize);
	*allocatedSpacePtr = allocatedSpace;
	allocatedSpace->lastFreeBlock = memorySpace.lastFreeBlock;
    allocatedSpace->firstFreeBlock = memorySpace.firstFreeBlock;
    allocatedSpace->spaceSize = memorySpace.spaceSize;
    allocatedSpace->spaceType = memorySpace.spaceType;
    allocatedSpace->spaceFlags = memorySpace.spaceFlags;
    allocatedSpace->spaceNumber = memorySpace.spaceNumber;
    allocatedSpace->rememberedSetSpaceNumber = memorySpace.rememberedSetSpaceNumber;
   
	if (allocatedSpace->firstFreeBlock > 0) {
		readFunction((unsigned char *) &allocatedSpace->space[0], allocatedSpace->firstFreeBlock * sizeof(oop), data);
	}

	if (!isPointerSpace(&memorySpace)) {
		if ((allocatedSpace->lastFreeBlock + 1) * sizeof(oop) < allocatedSpace->spaceSize) {
			readFunction((unsigned char *) &allocatedSpace->space[allocatedSpace->lastFreeBlock + 1], allocatedSpace->spaceSize - ((allocatedSpace->lastFreeBlock + 1) * sizeof(oop)), data);
		}
	}

	return memorySpace.spaceSize;
}

void relocateSpace(memorySpaceStruct *space, void *args)
{
	uint64_t *spaceNumber = (uint64_t *) args;

	if (isPointerSpace(space))
		relocatePointerSpace(space);
	else
		if (isStackSpace(space))
			relocateStackSpace(space);
		else
			relocateObjectSpace(space);

	space->spaceNumber = *spaceNumber;
	(*spaceNumber)++;
}

uint64_t loadImage(readFunctionType *readFunction, void *data, char *filename)
{ 
	imageHeaderStruct header;
	uint64_t spaceNumber = 0;

	if (readFunction((unsigned char *) &header, sizeof(imageHeaderStruct), data) == 0)
	{
		LOGW("Can't read image header");
		return 1;
	}

	if (header.magic != 0x4d495453) {
		LOGW("Bad magic number: %x", header.magic);
		return 2;
	}

	Development = header.development;

	readSpace (&Spaces[0], readFunction, data);
	EdenSpace = Spaces[0];

	readSpace (&Spaces[1], readFunction, data);
	SurvivorSpace1 = Spaces[1];
	if (isCurrentSpace(SurvivorSpace1)) {
		ActiveSurvivorSpace = SurvivorSpace1;
	} else {
		InactiveSurvivorSpace = SurvivorSpace1;
	}

	readSpace (&Spaces[2], readFunction, data);
	SurvivorSpace2 = Spaces[2];

	if (isCurrentSpace(SurvivorSpace2)) {
		ActiveSurvivorSpace = SurvivorSpace2;
	} else {
		InactiveSurvivorSpace = SurvivorSpace2;
	}

	readSpace (&Spaces[3], readFunction, data);
	RememberedSet = Spaces[3];

	readSpace (&Spaces[4], readFunction, data);
	WellKnownObjects = Spaces[4];	

	readSpace (&Spaces[5], readFunction, data);
	
	readSpace (&Spaces[6], readFunction, data);
	StackSpace = Spaces[6];
	
	readSpace (&Spaces[7], readFunction, data);
	OldSpace = Spaces[7];
	
	{
		uint64_t extraSpaceNumber = 8;

		while ((readSpace (&Spaces[extraSpaceNumber], readFunction, data)) > 0)
		{}
	}

	enumerateSpaces(relocateSpace, &spaceNumber);
	currentStackSpace = StackSpace;

	char sourcesFileName[256];
	char changesFileName[256];

	strncpy(ImageName, filename, 256);

	strncpy(sourcesFileName, filename, 256);
	strcpy (strrchr(sourcesFileName, '.'), ".sou");

	strncpy(changesFileName, filename, 256);
	strcpy (strrchr(changesFileName, '.'), ".cha");

	currentContext = ST_NIL;

	registerRememberedSetObject(ST_SYSTEM_CLASS);
	asSystemClass(ST_SYSTEM_CLASS)->imageName = CStringToST(filename);
	asSystemClass(ST_SYSTEM_CLASS)->sourceFileNames = newInstanceOfClass(ST_ARRAY_CLASS, 16, EdenSpace);
	instVarAtIntPut (asSystemClass(ST_SYSTEM_CLASS)->sourceFileNames, 0, CStringToST(sourcesFileName));	
	instVarAtIntPut (asSystemClass(ST_SYSTEM_CLASS)->sourceFileNames, 1, CStringToST(changesFileName));
	auditImage();
	return 0;
}

void launchImage(void)
{
	initializePrimitiveTable();

	StackSpace->lastFreeBlock = (StackSpace->spaceSize / sizeof(oop)) - 1;
	setupInterpreter(StackSpace);
	if (/* ST_START_CONTEXT != asOop(NULL) */ 0) {
		currentContext = ST_START_CONTEXT;
	}
	else {
		push (ST_START_OBJECT);
		dispatch (ST_START_SELECTOR, 0);
		if (Development)
			handleSocketCommands();
	}
}

void imageSizeHelper (memorySpaceStruct *space, void *arg)
{
	uint64_t *sizePointer = (uint64_t *) arg;
	*sizePointer += space->firstFreeBlock * sizeof(oop) + sizeof (memorySpaceStruct);
}

unsigned int imageSize(void)
{
	uint64_t size = sizeof(imageHeaderStruct);

	enumerateSpaces(imageSizeHelper, &size);

	return size;
}

void write64(uint64_t value, FILE *file)
{
	fwrite (&value, sizeof(uint64_t), 1, file);
}


void write32(uint32_t value, FILE *file)
{
	fwrite (&value, sizeof(uint32_t), 1, file);
}

void write16(uint16_t value, FILE *file)
{
	fwrite (&value, sizeof(uint16_t), 1, file);
}

void write8(uint8_t value, FILE *file)
{
	fwrite (&value, sizeof(uint8_t), 1, file);
}

void writeObject(oop object, FILE *fileStream)
{
	write64 (asObjectHeader(object)->size, fileStream);
	write64 ((uint64_t) asObjectHeader(object)->flags
			| asObjectHeader(object)->flips << 8
			| asObjectHeader(object)->numberOfNamedInstanceVariables << 16,
		fileStream);
	write64 (oopToOffset(classOf(object)), fileStream);
	write64 ((uint64_t) asObjectHeader(object)->identityHash, fileStream);

	uint64_t i;

	if (isBytes(object)) {
		for (i=0; i < (asObjectHeader(object)->size - sizeof(objectHeaderStruct) + sizeof (oop) - 1) / sizeof(oop); i++)
			write64 (instVarAtInt(object, i), fileStream);
		return;
	}

	for (i=0; i < (asObjectHeader(object)->size - sizeof(objectHeaderStruct))/sizeof(oop); i++)
		write64 (oopToOffset(instVarAtInt(object, i)), fileStream);
}

void writeObjectHeader(oop object, void *args)
{
	FILE *fileStream = (FILE *) args;

	objectHeaderStruct headerToWrite;
	
	headerToWrite.size = asObjectHeader(object)->size;
	headerToWrite.flags = asObjectHeader(object)->flags;
	headerToWrite.flips = asObjectHeader(object)->flips;
	headerToWrite.numberOfNamedInstanceVariables = asObjectHeader(object)->numberOfNamedInstanceVariables;
	headerToWrite.stClass = oopToOffset(asObjectHeader(object)->stClass);
	headerToWrite.identityHash = asObjectHeader(object)->identityHash;
	headerToWrite.bodyPointer = oopToOffset(asObjectHeader(object)->bodyPointer);

	fwrite (&headerToWrite, sizeof(objectHeaderStruct), 1, fileStream);
}

void objectToOffsets(oop object, __attribute__((unused)) void *args)
{
	uint64_t i;


	if (isBytes(object))
		return;
	
	if ((asObjectHeader(object)->flags & FREE) == FREE)
		return;

	for (i=0; i < (asObjectHeader(object)->size - sizeof(objectHeaderStruct))/sizeof(oop); i++) {
		uint64_t value = (uint64_t) oopToOffset(instVarAtInt(object, i));
		instVarAtIntPut(object, i, (oop) value);
	}
}

void objectFromOffsets(oop object, __attribute__((unused)) void *args)
{
	uint64_t i;


	if (isBytes(object))
		return;
	
	if ((asObjectHeader(object)->flags & FREE) == FREE)
		return;

	for (i=0; i < (asObjectHeader(object)->size - sizeof(objectHeaderStruct))/sizeof(oop); i++) {
		oop value = stPtrToC(instVarAtInt(object, i));
		instVarAtIntPut(object, i, (oop) value);
	}
}

void objectSpaceToOffsets (memorySpaceStruct *space)
{
	enumerateObjectsInSpace(space, objectToOffsets, NULL);
}

void objectSpaceFromOffsets (memorySpaceStruct *space)
{
	enumerateObjectsInSpace(space, objectFromOffsets, NULL);
}

void writeObjectSpace(memorySpaceStruct *space, FILE *fileStream)
{
	objectSpaceToOffsets(space);

	enumerateObjectsInSpace(space, writeObjectHeader, fileStream);

	fwrite (&space->space[space->lastFreeBlock + 1],
		1,
		space->spaceSize - ((space->lastFreeBlock + 1) * sizeof(oop)),
		fileStream);
	
	objectSpaceFromOffsets(space);
}

void writePointer(oop *pointer, void *args)
{
	FILE *fileStream = (FILE *)args;

	if ((*pointer == 0) || isImmediate(*pointer))
        write64(*pointer, fileStream);
	else
		write64(oopToOffset(*pointer), fileStream);
}

void writePointerSpace(memorySpaceStruct *space, FILE *fileStream)
{
	enumeratePointersInSpace(space, writePointer, fileStream);
}

void writeSpaceToFileStream(memorySpaceStruct *space, void *args)
{
	FILE *fileStream = (FILE *) args;
	write64 (space->spaceSize, fileStream);
	write64 (space->lastFreeBlock, fileStream);
	write64 (space->firstFreeBlock, fileStream);
	write16 (space->spaceType, fileStream);
	write16 (space->spaceNumber, fileStream);
	write16 (space->spaceFlags, fileStream);
	write16 (space->rememberedSetSpaceNumber, fileStream);

	if (isObjectSpace(space))
		writeObjectSpace(space, fileStream);
	else
		writePointerSpace(space, fileStream);
}

void saveImage(FILE *file)
{
    write32 (0x4d495453, file);
	write16 (0x0100, file);
	write16 (Development, file);
	write64 (imageSize(), file);

	uint64_t oldStackFirstFreeBlock = StackSpace->firstFreeBlock;

    StackSpace->firstFreeBlock = 0;
    WellKnownObjects->space[O_START_CONTEXT] = (oop) NULL;

    enumerateSpaces(writeSpaceToFileStream, file);

    StackSpace->firstFreeBlock = oldStackFirstFreeBlock;	
}

