// memory_primitives.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "object.h"

#define PRIM_AUDIT_IMAGE 300
#define PRIM_CREATE_OBJECT_HEADER_BACK_POINTERS 301
#define PRIM_SET_SYSTEM 302
#define PRIM_REALLOCATE_SPACE 303

int ExitOnAuditFail = 0;
#define exitIfNeeded() if (ExitOnAuditFail) exit(1)

void auditPointer(oop pointer, oop owner)
{
	if (!((pointer == 0) || isValidOop(pointer)))
		LOGI ("Audit: Invalid object pointer: %"PRIx64"  owner: %"PRIx64"", pointer, owner);
}

void auditObject(oop object, memorySpaceStruct *space, char *spaceName)
{
	if (isImmediate(object))
		return;

	if (isFree(object))
		return;

	if (isRelocated(object)) {
		LOGI ("Audit: Object %"PRIx64" relocated in space %s", object, spaceName);
		exitIfNeeded();
	}

	if (!isObjectInSpace(object, space)) {
		LOGI ("Audit: Object %"PRIx64" not in space %s", object, spaceName);
		exitIfNeeded();
	}

	if (asObjectHeader(object)->size > space->spaceSize) {
		LOGI ("Audit: Object %"PRIx64" too large %"PRIx64"", object, asObjectHeader(object)->size);
		exitIfNeeded();
	}
		
	if (asObjectHeader(object)->size < sizeof(objectHeaderStruct)) {
		LOGI ("Audit: Object %"PRIx64" too small %"PRIx64"", object, asObjectHeader(object)->size);
		exitIfNeeded();
	}

	if (isFree(object)) {
		LOGI ("Audit: Object %"PRIx64" is free %"PRIx64"", object, asObjectHeader(object)->size);
		exitIfNeeded();
	}

/*	if (isMarked(object)) {
		LOGI ("Audit: Object %"PRIx64" is marked %"PRIx64"", object, asObjectHeader(object)->size);
		exitIfNeeded();
	}
*/

	if (!isObjectInOldSpace(asObjectHeader(object)->stClass) && (!isObjectInAnyNewSpace(asObjectHeader(object)->stClass))) {
		LOGI ("Audit: Object %"PRIx64" class %"PRIx64" not in a valid space %"PRIx64, object, asObjectHeader(object)->stClass, asObjectHeader(object)->size);
		exitIfNeeded();
	}

	if (isFree(asObjectHeader(object)->stClass)) {
		LOGI ("Audit: Object %"PRIx64" class %"PRIx64" is free %"PRIx64"", object, asObjectHeader(object)->stClass, asObjectHeader(object)->size);
		exitIfNeeded();
	}

/*	if (isMarked(asObjectHeader(object)->stClass)) {
		LOGI ("Audit: Object %"PRIx64" class is marked %"PRIx64"", object, asObjectHeader(object)->size);
		exitIfNeeded();
	}
*/

	if ((asObjectHeader(object)->flags & 0xFF00) != 0) {
		LOGI ("Audit: Object %"PRIx64" bad flags %x", object, asObjectHeader(object)->flags);
		exitIfNeeded();
	}

	if (asObjectHeader(object)->numberOfNamedInstanceVariables > 64) {
		LOGI ("Audit: Too many named inst vars %"PRIx64" %"PRIx64"", (uint64_t) object, (uint64_t) asObjectHeader(object)->numberOfNamedInstanceVariables);
		exitIfNeeded();
	}

	if (!isObjectInActiveMemorySpace(asObjectHeader(object)->stClass) ) {
		LOGI ("Audit: Object class bad %"PRIx64" class %"PRIx64"", (uint64_t) object, (uint64_t)asObjectHeader(object)->stClass);
		exitIfNeeded();
	}

/*	if (!isBodyInSpace(object, space) ) {
		LOGI ("Audit: Object body %"PRIx64" not in space %s", object, spaceName);
		exitIfNeeded();
	}
*/

	if (!(isObjectInStackSpace(object)) && (asObjectHeader(object)->bodyPointer != 0) && (bodyHeaderPointer(object) != object)) {
		LOGI ("Audit: Object body pointer %"PRIx64" doesn't point to object %"PRIx64, bodyHeaderPointer(object),object);
		exitIfNeeded();
	}

	if (!isBytes(object)) {
		int instVarNumber;
		for (instVarNumber = 0; instVarNumber < totalObjectSize(object); instVarNumber++) {
			oop instVarObject = instVarAtInt(object, instVarNumber);
			if (isFree(object))
				LOGI ("Free object %" PRIx64 " connected to used object %" PRIx64, instVarObject, object);
			if (isObjectInOldSpace(object) && (isObjectInAnyNewSpace(instVarObject))) {
				if (!findRememberedSetObject(object))
					LOGI ("Audit: Object not in remembered set %" PRIx64" %" PRIx64, object, instVarObject);
			}
			auditPointer(instVarObject, object);
		}
	}
}

void auditObjectSpace(memorySpaceStruct *space, char *spaceName)
{
	oop object;

	for (object = (oop) &space->space[0];
			object < (oop) &space->space[space->firstFreeBlock];
			object += sizeof (objectHeaderStruct))
	{
		auditObject(object, space, spaceName);
	}
}

void auditStackSpace(oop context, memorySpaceStruct *space, char *spaceName)
{
/*	oop object;

	for (object = (oop) &space->space[space->lastFreeBlock + 1];
			object < (oop)&space->space[spaceSize(space) - 1];
			object += sizeof (objectHeaderStruct))
	{
		auditObject(object, space, spaceName);
	}
*/
	oop frame;
	if (context == ST_NIL)
		return;

	if (oopPtr(context) == NULL)
		return;

	for (frame = context;
		frame != ST_NIL;
		frame = asContext(frame)->frame) {

		auditObject(frame, space, spaceName);
	}
}

void auditPointerSpace(memorySpaceStruct *space)
{
	uint64_t i;

	for (i = 0; i < space->firstFreeBlock; i++)
	{
		auditPointer((oop) space->space[i], (oop) space);
	}
}

void auditBackPointers(memorySpaceStruct *space)
{
	uint64_t *bodyBackPointer;

// loop backwards through bodies from top of space towards bottom
	for( bodyBackPointer = &space->space[spaceSize(space) - 1];
		bodyBackPointer > &space->space[space->lastFreeBlock];
		bodyBackPointer -= totalObjectSize(*bodyBackPointer) + 1)
		{
			if (asObjectHeader(*bodyBackPointer)->bodyPointer == 0) {
				LOGI ("Audit backpointers: Object has empty body: %" PRIx64, *bodyBackPointer);
				return;
			}

			// Find the header for this body
			if (!isObjectInSpace(*bodyBackPointer, space)) {
				LOGI ("Audit backpointers: Object %" PRIx64 " isn't in the same space %" PRIx64,
					asOop(*bodyBackPointer), asOop(space));
				return;
			}
		}
}

extern int EdenUsedForGC;

void auditImage()
{
	if (!EdenUsedForGC)
		auditObjectSpace(EdenSpace, "Eden Space");
	auditObjectSpace(SurvivorSpace1, "Survivor Space 1");
	auditObjectSpace(SurvivorSpace2, "Survivor Space 2");
	auditObjectSpace(OldSpace, "Old Space");
	auditStackSpace(currentContext, currentStackSpace, "Stack Space");
	auditPointerSpace(RememberedSet);
	auditPointerSpace(WellKnownObjects);
	auditBackPointers(EdenSpace);
	auditBackPointers(SurvivorSpace1);
	auditBackPointers(SurvivorSpace2);
	auditBackPointers(OldSpace);
}

void debugAuditImage()
{
	auditImage();
	exitIfNeeded();
}

void primAuditImage()
{
	auditImage();
	push (cIntToST(0));
	push (cIntToST(0));
}

void primReallocateObjectSpaces()
{
	reallocateSpace(0, EdenSpace->spaceSize);
	reallocateSpace(1, SurvivorSpace1->spaceSize);
	reallocateSpace(2, SurvivorSpace2->spaceSize);
	reallocateSpace(7, OldSpace->spaceSize);	
	auditImage();
}

void primSetSystem()
{
	oop receiverOop = getReceiver();
	WellKnownObjects->space[O_SYSTEM_CLASS] = receiverOop;	

	push (cIntToST(0));
	push (cIntToST(0));
}

void primReallocateObjectSpace()
{
	oop spaceNumberOop = getLocal(0);
	oop sizeOop = getLocal(1);

	int64_t spaceNumber;
    int64_t size;

    if (!isSmallInteger(spaceNumberOop)) {
		push (cIntToST(1));
		push (getReceiver());
	}

	spaceNumber = stIntToC(spaceNumberOop);

    if (spaceNumber < 0 || spaceNumber > 255) {
		push (cIntToST(2));
		push (getReceiver());
	}

    if (!isSmallInteger(sizeOop)) {
		push (cIntToST(3));
		push (getReceiver());
	}

	size = stIntToC(sizeOop);

    if (size < 0) {
		push (cIntToST(4));
		push (getReceiver());
	}

	reallocateSpace((int)spaceNumber, size);
	auditImage();

	push (cIntToST(0));
	push (cIntToST(0));
}

void initializeMemoryPrimitives()
{
	primitiveTable[PRIM_AUDIT_IMAGE] = primAuditImage;
	primitiveTable[PRIM_CREATE_OBJECT_HEADER_BACK_POINTERS] = primReallocateObjectSpaces;
	primitiveTable[PRIM_SET_SYSTEM] = primSetSystem;	
	primitiveTable[PRIM_REALLOCATE_SPACE] = primReallocateObjectSpace;
}
