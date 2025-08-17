// remote.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "object.h"
#include "websockets.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

char *nextToken;
int listenfd = 0, connfd = 0;
unsigned long bytes = 0;
oop debugOop;

#define REMOTE_BUFFER_SIZE 16384
char recvBuff[REMOTE_BUFFER_SIZE+1];
char sendBuff[REMOTE_BUFFER_SIZE+1];

void run(void)
{
	basicInterpret(0);
	simlog ("Finished running");
}

void spaces(void)
{
	simlog ("EdenSpace: %"PRIx64 " - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n", asOop(EdenSpace),
		asOop(&EdenSpace->space[EdenSpace->spaceSize / sizeof(oop)]), EdenSpace->spaceSize, EdenSpace->lastFreeBlock);
	simlog ("SurvivorSpace1: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n", asOop(SurvivorSpace1),
		asOop(&SurvivorSpace1->space[SurvivorSpace1->spaceSize / sizeof(oop)]), SurvivorSpace1->spaceSize, SurvivorSpace1->lastFreeBlock);
	simlog ("SurvivorSpace2: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n", asOop(SurvivorSpace2),
		asOop(&SurvivorSpace2->space[SurvivorSpace2->spaceSize / sizeof(oop)]), SurvivorSpace2->spaceSize, SurvivorSpace2->lastFreeBlock);
	simlog ("RememberedSet: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n", asOop(RememberedSet),
		asOop(&RememberedSet->space[RememberedSet->spaceSize / sizeof(oop)]), RememberedSet->spaceSize, RememberedSet->lastFreeBlock);
	simlog ("OldSpace: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n", asOop(OldSpace),
		asOop(&OldSpace->space[OldSpace->spaceSize / sizeof(oop)]), OldSpace->spaceSize, OldSpace->lastFreeBlock);
	simlog ("WellKnownObjects: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n",
		asOop(WellKnownObjects), asOop(&WellKnownObjects->space[WellKnownObjects->spaceSize / sizeof(oop)]), WellKnownObjects->spaceSize, WellKnownObjects->lastFreeBlock);
	simlog ("StackSpace: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n",
		asOop(StackSpace), asOop(&StackSpace->space[StackSpace->spaceSize / sizeof(oop)]), StackSpace->spaceSize, StackSpace->lastFreeBlock);
	simlog ("ActiveSurvivorSpace: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n",
		asOop(ActiveSurvivorSpace), asOop(&ActiveSurvivorSpace->space[ActiveSurvivorSpace->spaceSize / sizeof(oop)]), ActiveSurvivorSpace->spaceSize, ActiveSurvivorSpace->lastFreeBlock);
	simlog ("InactiveSurvivorSpace: %"PRIx64" - %"PRIx64" size %"PRIx64" lastFreeBlock %"PRIx64"\n",
		asOop(InactiveSurvivorSpace), asOop(&InactiveSurvivorSpace->space[InactiveSurvivorSpace->spaceSize / sizeof(oop)]), InactiveSurvivorSpace->spaceSize, InactiveSurvivorSpace->lastFreeBlock);
}

void debugSpaces(void)
{
	logPtr = logString;
	*logPtr = '\0';
	spaces();
	printf ("%s", logString);
}

void gdbDebugSpaces(void)
{
	debugSpaces();
}

void showString (oop p)
{
	char *displayString;
	int i;
	displayString = malloc((size_t) basicByteSize(p)+1);

	for (i=1; i <= basicByteSize(p); i++)
		displayString[i-1] = basicByteAtInt(p, i);

	displayString[basicByteSize(p)] = '\0';

	simlog ("%s", displayString);
	free (displayString);
}

void showOop(oop p)
{
	if (p == ST_NIL)
		simlog ("nil");

	else if (p == ST_TRUE)
		simlog ("true");

	else if (p == ST_FALSE)
		simlog ("false");

	else if (isSmallInteger(p))
		simlog ("%" PRId64, stIntToC(p));

	else if (isCharacter(p))
		simlog ("%c", (char) stIntToC(p));

	else if (isFloat(p))
        simlog ("%lf", (double) stFloatToC(p));

	else if (isContextPointer(p))
        simlog ("context pointer %" PRIx64, (uint64_t) p);

	else if ((p >= asOop(&StackSpace->space[0])) && (p <= asOop(&StackSpace->space[StackSpace->spaceSize / sizeof(oop)]))) {
		simlog ("stack pointer");
		}

	else if (!isValidOop(p))
		simlog ("invalid oop %"PRIx64, (uint64_t) p);

	else if (classOf(p) == ST_BYTE_STRING_CLASS)
	{
		showString(p);
	}
	else if (classOf(p) == ST_BYTE_SYMBOL_CLASS)
	{
		simlog("#");
		showString(p);
	}
	else if (classOf(p) == ST_METACLASS_CLASS)
	{
		simlog ("metaclass ");
		showString (asClass(asMetaclass(p)->thisClass)->name);
	}
	else if (classOf(classOf(p)) == ST_METACLASS_CLASS)
	{
		simlog ("class ");
		if (asClass(p) -> name == ST_NIL)
			simlog ("<nil class>");
		else
			showString (asClass(p) -> name);
	}
	else {
		simlog ("a ");
		if (asClass(classOf(p)) -> name == ST_NIL)
			simlog ("<nil class>");
		else
			showString (asClass(classOf(p))->name);
	}
}

void debugShowOop(oop p)
{
	logPtr = logString;
	*logPtr = '\0';
	showOop(p);
	LOGI("%s", logString);
}

void gdbShowOop(void)
{
	debugShowOop(debugOop);
}

void showMethodSignature(oop p)
{
	if (classOf(p) == ST_COMPILED_BLOCK_CLASS) {
		simlog ("Block");
		return;
	}

	oop methodOop = p;
	oop mclassOop = asCompiledMethod(methodOop)->mclass;

	oop methodDictionaryOop = asBehavior(mclassOop)->methodDictionary;
	showOop (mclassOop);
	simlog(" >> ");

	oop key = identityDictionaryKeyAtValue (methodDictionaryOop, p);
	showString(key);
}

void showStackFrame(contextStruct *context)
{
	oop stackPtr;
	char *argument;

	if (context->frame == ST_NIL)
	{
		simlog ("Frame is null\n");
		return;
	}

	uint64_t frameNumber = 0;
	argument = strtok_r(NULL, " ", &nextToken);
	if (argument)
		sscanf (argument, "%"PRIx64"", &frameNumber);

	if (frameNumber > 100) {
		simlog("Invalid frame number\n");
		return;		
	}

	for ( ; frameNumber > 0; frameNumber--) {
		if (context->frame == ST_NIL) {
			simlog("Top of stack\n");
			return;
		}
		context = asContext(context->frame);
	}

	for (stackPtr = asOop(&asContext(context->frame)->stackBody[stIntToC(asContext(context->frame)->stackOffset)]);
		stackPtr < asOop(&context->stackBody[stIntToC(context->stackOffset)]);
		stackPtr += sizeof(oop))
	{
		if (stackPtr == asOop(&context->frame))
			simlog ("%"PRIx64":  ~%"PRIx64" (frame pointer)\n", stackPtr, asOop(*oopPtr(stackPtr)));
		else if (stackPtr == asOop(&context->stackOffset))
			simlog ("%"PRIx64":  ~%"PRIx64" (stack offset)\n", stackPtr, asOop(*oopPtr(stackPtr)));
		else if (stackPtr == asOop(&context->pcOffset))
			simlog ("%"PRIx64":  ~%"PRIx64" (pcOffset) [%"PRIx64"]\n", stackPtr, asOop(*oopPtr(stackPtr)), stIntToC(*oopPtr(stackPtr)));
		else if (stackPtr == asOop(&context->method)) {
			simlog ("%"PRIx64":  ~%"PRIx64" (method) <", stackPtr, asOop(*oopPtr(stackPtr)));
			showMethodSignature (asOop(*oopPtr(stackPtr)));
			simlog (">\n");
		}
		else if (stackPtr == asOop(&context->methodContext))
			simlog ("%"PRIx64":  ~%"PRIx64" (methodContext)\n", stackPtr, asOop(*oopPtr(stackPtr)));
		else if (stackPtr == asOop(&context->contextId))
			simlog ("%"PRIx64":  ~%"PRIx64" (contextId)\n", stackPtr, asOop(*oopPtr(stackPtr)));
		else {
		simlog ("%"PRIx64":  ~%"PRIx64"  <", stackPtr, asOop(*oopPtr(stackPtr)));
			showOop (*((oop*) stackPtr));
			simlog(">\n");
		}
	}
	
	dumpWalkback("Stack dump");
//	LOGI ("Walkback String:\n%s", walkbackDump);
//	simlog ("stack dump\n%s", walkbackDump);
}

oop findInstVarName(oop classObject, int i)
{
	oop result = cIntToST(0);
	if (asBehavior(classObject)->superclass != ST_NIL)
		result = findInstVarName(asBehavior(classObject)->superclass, i);

	if (!isSmallInteger(result))
		return result;

	uint64_t parentCount = stIntToC(result);
	uint64_t instSize = totalObjectSize(asBehavior(classObject)->instVarNames);

	if (i < (parentCount + instSize))
		return (instVarAtInt(asBehavior(classObject)->instVarNames, i - parentCount));

	return cIntToST(stIntToC(result) + totalObjectSize(asBehavior(classObject)->instVarNames));
}

void inspectObject()
{
	oop address = 0;
	oop objectOop;
	char *argument;

	argument = strtok_r(NULL, " ", &nextToken);
	if (argument)
		sscanf (argument, "%"PRIx64"", &address);

	if (address == 0) {
		simlog ("Invalid address\n");
		return;
	}

	objectOop = address;
	if (!isObjectInObjectSpace(objectOop)) {
		simlog ("Invalid address\n");
		return;
	}

	simlog ("%"PRIx64":  ~%"PRIx64"  size: %"PRIx64"\n", objectOop, memorySize(objectOop), memorySize(objectOop));
	simlog ("%"PRIx64":  ~%"PRIx64"  flags: %x flips: %d namedVars: %d\n", address + 8, *(oopPtr(address + 8)),
			asObjectHeader(objectOop)->flags, asObjectHeader(objectOop)->flips, asObjectHeader(objectOop)->numberOfNamedInstanceVariables);
	simlog ("%"PRIx64":  ~%"PRIx64" ", address + 16, *(oopPtr(address + 16)));
	simlog("   <");
	showOop (classOf(objectOop));
	simlog(">\n");
	simlog ("%"PRIx64":  ~%"PRIx64"  identityHash: %"PRIx64"\n", address + 24, *(oopPtr(address + 24)), asObjectHeader(objectOop)->identityHash);

	simlog ("%"PRIx64":  ~%"PRIx64"  bodyPointer: %"PRIx64"\n", address + 32, *(oopPtr(address + 32)), asObjectHeader(objectOop)->bodyPointer);

	int i;
	for (i=0; i<asObjectHeader(objectOop)->numberOfNamedInstanceVariables; i++) {
		simlog ("%"PRIx64":  ~%"PRIx64, (uint64_t) &instVarAtInt(objectOop,i), (uint64_t) instVarAtInt(objectOop,i));
		simlog("   <");
		showOop (instVarAtInt(objectOop,i));
		simlog(">  (");
		showString(findInstVarName(classOf(objectOop), i));
		simlog (")\n");
	}
	if(isBytes(objectOop)) {
	for (i=1; i <= ((basicByteSize(objectOop) + sizeof(oop) - 1) / sizeof(oop)); i++) {
        oop var = indexedVarAtInt(objectOop, i);
        simlog ("%"PRIx64":  %"PRIx64" \n", (uint64_t) &instVarAtInt(objectOop,i+asObjectHeader(objectOop)->numberOfNamedInstanceVariables), var);
        }
	}
	else
		for (i=1; i <= indexedObjectSize(objectOop); i++) {
			oop var = indexedVarAtInt(objectOop, i);
			simlog ("%"PRIx64":  ~%"PRIx64" ", (uint64_t) &instVarAtInt(objectOop,i-1+asObjectHeader(objectOop)->numberOfNamedInstanceVariables), var);
			simlog("   <");
			showOop (var);
			simlog(">\n");
		}
}

void dumpThisContext()
{
	oop *p = (oop *) currentContext;

	simlog ("%"PRIx64":  ~%"PRIx64" (frame)\n", asOop(p), asOop(*p));
	p++;
	simlog ("%"PRIx64":  ~%"PRIx64" (stack)\n", asOop(p), asOop(*p));
	p++;
	simlog ("%"PRIx64":  ~%"PRIx64" (pc) [%"PRIx64"]\n",
		asOop(p), asOop(*p),
		(uint64_t) (p - asOop(asCompiledMethod(asContext(currentContext)->method)->bytecodes)));
	p++;
	simlog ("%"PRIx64":  ~%"PRIx64" (method)\n", asOop(p), asOop(*p));
	p++;
	simlog ("%"PRIx64":  ~%"PRIx64" (methodContext)\n", asOop(p), asOop(*p));
	p++;
	simlog ("%"PRIx64":  ~%"PRIx64" (contextId)\n", asOop(p), asOop(*p));
	p++;
}

#define cprint(x) (((x)>=32 && (x)<=126) ? (x) : '.')
void dump()
{
	oop address = 0;
	oop *object;
	uint32_t i;

	char *argument;
	argument = strtok_r(NULL, " ", &nextToken);
	if (argument) {
		sscanf (argument, "%"PRIx64, &address);
	}

	if (address == 0) {
		simlog ("Invalid address\n");
		return;
	}
	
	object = oopPtr(address);
	for (i=0; i < 16; i++)
	{
		unsigned char *c = (unsigned char *) &object[i];
		unsigned char byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
		byte1 = *c++;
		byte2 = *c++;
		byte3 = *c++;
		byte4 = *c++;
		byte5 = *c++;
		byte6 = *c++;
		byte7 = *c++;
		byte8 = *c++;
		simlog ("%"PRIx64":  %02x %02x %02x %02x %02x %02x %02x %02x   %c%c%c%c%c%c%c%c\n", asOop(&object[i]),
			byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8,
			cprint(byte1), cprint(byte2), cprint(byte3), cprint(byte4),
			cprint(byte5), cprint(byte6), cprint(byte7), cprint(byte8));
	}
}

void dumpBytecode(uint64_t numberOfBytes, uint64_t offset, uint8_t bytecode)
{
	char bytecodeName[256];
	oop bytecodeTableOop = ST_BYTECODE_TABLE;
	STStringToC(instVarAtInt(bytecodeTableOop, bytecode * 3), bytecodeName);

	switch (numberOfBytes) {
		case 1:
			simlog("%" PRIx64 " Bytecode: %02x %s ", offset, bytecode, bytecodeName);
			break;
		case 2:
			simlog("%" PRIx64 " Bytecode: %02x %02x %s ", offset, bytecode, peekBytecode1(), bytecodeName);
			break;
		case 3:
			simlog("%" PRIx64 " Bytecode: %02x %02x %02x %s ", offset, bytecode, peekBytecode1(),  peekBytecode2(), bytecodeName);
			break;
		case 4:
			simlog("%" PRIx64 " Bytecode: %02x %02x %02x %02x %s ", offset, bytecode, peekBytecode1(), peekBytecode2(), peekBytecode3(), bytecodeName);
			break;
		case 5:
			simlog("%" PRIx64 " Bytecode: %02x %02x %02x %02x %02x %s ", offset, bytecode, peekBytecode1(),  peekBytecode2(),  peekBytecode3(),  peekBytecode4(), bytecodeName);
			break;
		default:
			break;
	}

}

void dumpBytecodeArguments(oop argumentArray, uint64_t numberOfBytes, uint64_t offset)
{
	int argumentSpecSize = totalObjectSize(argumentArray);
	int argPointer;
	char argumentType[256], argumentDescription[256];
	argumentDescription[0] = '\0';

	for (argPointer = 2; argPointer < argumentSpecSize; argPointer++)
	{
		char argDescription[256];
		oop argumentTypeOop = instVarAtInt(argumentArray, argPointer);

		STStringToC(argumentTypeOop, argumentType);

		if (strncmp(argumentType, "integerImplicit", 256) == 0) {
			argPointer++;
			oop integer = instVarAtInt(argumentArray, argPointer);
			showOop(integer);
			}

		if (strncmp(argumentType, "integerOneByte", 256) == 0) {
			int8_t integer = peekBytecode1();
			showOop(cIntToST(integer));
			}

		if (strncmp(argumentType, "literalImplicit", 256) == 0) {
			oop literal;
			argPointer++;
			uint64_t literalNumber = stIntToC(instVarAtInt(argumentArray, argPointer));
			literal = getLiteral(asContext(currentContext)->method, literalNumber);
			showOop(literal);
			}

		if (strncmp(argumentType, "globalImplicit", 256) == 0) {
			oop literal;
			argPointer++;
			uint64_t literalNumber = stIntToC(instVarAtInt(argumentArray, argPointer));
			literal = getLiteral(asContext(currentContext)->method, literalNumber);
			showOop(asAssociation(literal)->key);
			}

		if (strncmp(argumentType, "localImplicit", 256) == 0) {
			argPointer++;
			uint64_t localNumber = stIntToC(instVarAtInt(argumentArray, argPointer));
			
			oop localName = indexedVarAtInt(asCompiledMethod(asContext(currentContext)->method)->localVariableNames, localNumber + 1);
			showOop(localName);
			}

		if (strncmp(argumentType, "oneByteWellKnown", 256) == 0) {
			uint64_t wellKnownIndex = peekBytecode1();
			oop specialSelector = specialSelectors(wellKnownIndex);
			showOop(specialSelector);
			}

		if (strncmp(argumentType, "offsetOneByte", 256) == 0) {
			int8_t jumpOffset = offset + numberOfBytes + peekBytecode1();
			simlog("%x", jumpOffset);
			}

		if (strncmp(argumentType, "instVarImplicit", 256) == 0) {
			argPointer++;
			uint64_t instVarNumber = stIntToC(instVarAtInt(argumentArray, argPointer));
			showString(findInstVarName(classOf(getReceiver()), instVarNumber));
			}
	}

	simlog("\n\n");
}

void dumpMethodSource(uint64_t offset)
{
	int result;

	oop sourceOffsetsArray = asCompiledMethod(asContext(currentContext)->method)->sourceOffsets;
	uint64_t sourceFileNumber = stIntToC(indexedVarAtInt(sourceOffsetsArray, 1));
	uint64_t sourceStart = stIntToC(indexedVarAtInt(sourceOffsetsArray, 2));
	uint64_t sourceEnd = stIntToC(indexedVarAtInt(sourceOffsetsArray, 3));

	char imageName[256];
	char sourceFileName[256];

	oop systemClassOop = ST_SYSTEM_CLASS;
	STStringToC (asSystemClass(systemClassOop)->imageName, imageName);

	oop sourceFileNameOop = instVarAtInt(asSystemClass(systemClassOop)->sourceFileNames, sourceFileNumber - 1);
	STStringToC (sourceFileNameOop, sourceFileName);

	FILE *changesFile = fopen((const char *) sourceFileName, "r");
	if (changesFile == NULL) { 
		LOGI ("Can't open changes file '%s'", sourceFileName);
		simlog("Can't open changes file '%s'", sourceFileName);
		return;
	}
	

	if ((result = fseek (changesFile, sourceStart, SEEK_SET)) != 0) {
		fclose(changesFile);
		return;
	}

	int highlightStart = -1, highlightEnd = -1, sourceOffsetsIndex;

	for (sourceOffsetsIndex = 4 ;
				sourceOffsetsIndex < indexedObjectSize(sourceOffsetsArray);
				sourceOffsetsIndex += 3) {

		if (offset == stIntToC(indexedVarAtInt (sourceOffsetsArray,  sourceOffsetsIndex))) {
			highlightStart = stIntToC(indexedVarAtInt (sourceOffsetsArray,  sourceOffsetsIndex + 1));
			highlightEnd = stIntToC(indexedVarAtInt (sourceOffsetsArray,  sourceOffsetsIndex + 2));
			}
		}

	int i, j, len;
	char *stSource, *stSourceEnd;
	stSource = malloc(sourceEnd - sourceStart + 8);
	fread (stSource, 1, sourceEnd - sourceStart, changesFile);
	stSource[sourceEnd - sourceStart] = '\0';
	len = strlen(stSource);
	   	
  	for(i = 0; i < len; i++)
	{
		if(stSource[i] == '\r')
		{
			for(j = i; j < len; j++)
			{
				stSource[j] = stSource[j + 1];
			}
			len--;
			i--;	
		} 
	}

	if (highlightStart >= 0) {
		memmove (stSource + highlightEnd + 3, stSource + highlightEnd + 1, strlen(stSource + highlightEnd + 1) + 1);
		stSource[highlightEnd + 1] = '<';
		stSource[highlightEnd + 2] = '<';

		memmove (stSource + highlightStart + 2, stSource + highlightStart, strlen(stSource + highlightStart) + 1);
		stSource[highlightStart] = '>';
		stSource[highlightStart + 1] = '>';
	}
	else
		stSourceEnd = stSource + strlen(stSource);

	simlog("%s", stSource);
	free(stSource);
	
	fclose(changesFile);
}

void logBytecode()
{
	char string[256];
	int argPointer;

	uint8_t bytecode1 = peekBytecode();

	uint64_t offset = stIntToC(asContext(currentContext)->pcOffset);

	oop bytecodeTableOop = ST_BYTECODE_TABLE;

	oop argumentArray = instVarAtInt(bytecodeTableOop, bytecode1 * 3 + 2);
	uint64_t numberOfBytes = stIntToC(instVarAtInt(argumentArray, 1));

	dumpBytecode(numberOfBytes, offset, bytecode1);
	dumpBytecodeArguments(argumentArray, numberOfBytes, offset);
	dumpMethodSource(offset);
}

void sendByteCode()
{
	basicInterpret(1);
	logBytecode();
}

void step()
{
	oop oldStopFrame = stopFrame;
	stopFrame = currentContext;
	basicInterpret(0);
	stopFrame = oldStopFrame;
	logBytecode();
}

void developmentOff()
{
	Development = 0;
}

char *readArgument (char *buff, int *offset)
{
	char c, *start, *original, *quoted;

	c = buff[(*offset)++];

	if (c != '{')
		return NULL;

	start = original = quoted = &buff[(*offset)++];
	while (*original != '}') {
		if (*original == '\0')
			return NULL;
		if (*original == '\\') {
			original++;
			(*offset)++;
		}

		*quoted++ = *original++;
		(*offset)++;
	}
	*quoted = '\0';
	return start;
}

oop stringDictionaryLookup(oop dictionary, oop object)
{
	oop objects = asDictionary(dictionary)->values;

	oop lookupKeyOop = object;
	int i, j;
	oop foundAssociation = ST_NIL;

	for (i=1; i <= indexedObjectSize (objects); i++)
	{
		oop value = indexedVarAtInt(objects, i);
		if (value != 0 && value != ST_NIL) {
			oop associationOop = value;
			oop keyOop = asAssociation(associationOop)->key;
			if (basicByteSize(keyOop) == basicByteSize(lookupKeyOop)) {
				foundAssociation = associationOop;
				for (j=1; j <= basicByteSize(keyOop); j++) {
					if (basicByteAtInt(lookupKeyOop, j) != basicByteAtInt(keyOop, j)) {
						foundAssociation = ST_NIL;
						break;
					}
				}
				if (foundAssociation != ST_NIL)
					return foundAssociation;
			}
		}

	}
	return ST_NIL;
}

void evaluate(char *code)
{
	oop codeString = CStringToST(code);
	oop oldStopFrame = stopFrame;
	stopFrame = currentContext;
	dispatchSpecial1(SPECIAL_EVALUATE, ST_SMALLTALK_PARSER_CLASS, codeString);
	basicInterpret(0);
	stopFrame = oldStopFrame;
}

void debug(char *code)
{
	oop codeString = CStringToST(code);
	oop oldStopFrame = stopFrame;
	stopFrame = currentContext;
	dispatchSpecial1(SPECIAL_DEBUGIT, ST_SMALLTALK_PARSER_CLASS, codeString);
	basicInterpret(0);
	stopFrame = oldStopFrame;
}

void printTopOfStack()
{
	oop oldStopFrame = stopFrame;
	stopFrame = currentContext;
	dispatchSpecial0(0x0f, pop ());
	basicInterpret(0);
	stopFrame = oldStopFrame;
}

EMSCRIPTEN_KEEPALIVE void evaluateJsonCommands(char *buff)
{
	oop codeString = CStringToST(buff);
	oop oldStopFrame = stopFrame;
	stopFrame = currentContext;
	dispatchSpecial1(SPECIAL_EVALUATE_JSON, ST_JSON_PARSER_CLASS, codeString);
	basicInterpret(0);
	stopFrame = oldStopFrame;
}

EMSCRIPTEN_KEEPALIVE char *processCommand(char *buff)
{
	char *token = buff;

	logPtr = &logString[0];
	*logPtr = '\0';

	if (buff[0] == '[') {
		evaluateJsonCommands(buff);
		return "Done";
	}
	
	token = strtok_r(buff, " ", &nextToken);
	if (token == 0)
		return "";

	if (strcmp (token, "run") == 0)
		run();

	if (strcmp (token, "spaces") == 0)
		spaces();

	if (strcmp (token, "stack") == 0) {
		showStackFrame(asContext(currentContext));
		snprintf (sendBuff, REMOTE_BUFFER_SIZE, "%s", logString);
		return sendBuff;
	}

	if (strcmp (token, "step") == 0)
		step();

	if (strcmp (token, "send") == 0)
		sendByteCode();

	if (strcmp (token, "peek") == 0)
		logBytecode();

	if (strcmp (token, "inspect") == 0)
		inspectObject();

	if (strcmp (token, "dump") == 0)
		dump();

	if (strcmp (token, "close") == 0){
		close(connfd);
		exit(0);
	}

	if (strcmp (token, "thisContext") == 0)
		dumpThisContext();

	if (strcmp (token, "developmentOff") == 0)
		developmentOff();

	if (strcmp (token, "evaluate") == 0)
		evaluate(nextToken);

	if (strcmp (token, "doit") == 0) {
		evaluate(nextToken);
		pop ();
	}

	if (strcmp (token, "printit") == 0) {
		oop string;
		evaluate(nextToken);
		printTopOfStack();
		string = pop ();
		showOop(string);
	}

	if (strcmp (token, "debugit") == 0) {
		oop string;
		debug(nextToken);
		showOop(ST_NIL);
	}

	snprintf (sendBuff, REMOTE_BUFFER_SIZE, "%s", logString);
	return sendBuff;
}

void handleSocketCommands(void)
{
	LOGI ("Waiting for a socket command");
	while (1)
	{
		bytes = receiveWSMessage (connfd, recvBuff, REMOTE_BUFFER_SIZE);
		if (bytes == 0)
			break;
		recvBuff[bytes] = '\0';
		sendBuff[0] = '\0';
		processCommand(recvBuff);
		logPtr = &logString[0];
		*logPtr = '\0';
		sendWSMessage(connfd, sendBuff, strlen(sendBuff));
	}
}

void setupRemoteSocket(void)
{
	struct sockaddr_in serv_addr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    LOGI ("listenfd %d %d", listenfd, errno);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(debugWebSocketPort);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

	if (acceptWebSocketConnection(connfd) == 0)
		LOGI("Web socket connected");
}

void startupDebugger(void)
{
	setupRemoteSocket();
	handleSocketCommands();
}

void terminateSocket()
{
	close(connfd);
}

