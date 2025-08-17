// object.h
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define VERSION_NAME "0.1"

#define WALKBACK(...) do    {printf (__VA_ARGS__); printf ("\n");  fflush (stdout);} while (0)

#define LOGI(...) do {printf (__VA_ARGS__); printf ("\n");} while (0)
#define LOGW(...) do {printf (__VA_ARGS__); printf ("\n");} while (0)
#define LOGE(...) do    {printf (__VA_ARGS__); printf ("\n");} while (0)

#define oopPtr(x) ((oop *)(x))
#define asOop(x) ((oop)(x))

#define FALSE 0
#define TRUE 1

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(a) ((a)<0?-(a):(a))
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wempty-body"

typedef uint64_t oop;

#define MAX_SPACES 256
typedef struct {
	uint64_t spaceSize;
	uint64_t lastFreeBlock;
	uint64_t firstFreeBlock;
	uint16_t spaceType;

// spaceType definitions
#define EDEN_SPACE 0
#define SURVIVOR_SPACE_1 1
#define SURVIVOR_SPACE_2 2
#define REMEMBERED_SET 3
#define WELL_KNOWN_OBJECTS_SPACE 4
#define OLD_SPACE 5
#define STACK_SPACE 6

	uint16_t spaceNumber;
	uint16_t spaceFlags;
#define SPACE_HAS_TOP_HEADERS 1
#define SPACE_IS_OBJECT_SPACE 2
#define SPACE_IS_POINTER_SPACE 4
#define SPACE_IS_SCAVENGED 8
#define SPACE_IS_STACK_MANAGED 16
#define SPACE_IS_MARK_SWEEP_MANAGED 32
#define SPACE_HAS_SPACE_OBJECT 64
#define SPACE_IS_CURRENT_SPACE 128

	uint16_t rememberedSetSpaceNumber;
	oop space[];
} memorySpaceStruct;

#define asMemorySpace(x) ((memorySpaceStruct *)(x))
#define spaceSize(x) ((((memorySpaceStruct *)(x))->spaceSize) / sizeof(oop))
#define endOfSpace(x) &((x)->space[x->spaceSize / sizeof(oop)])
#define isObjectSpace(x) ((asMemorySpace(x)->spaceFlags & SPACE_IS_OBJECT_SPACE) == SPACE_IS_OBJECT_SPACE)
#define isCurrentSpace(x) ((asMemorySpace(x)->spaceFlags & SPACE_IS_CURRENT_SPACE) == SPACE_IS_CURRENT_SPACE)
#define isActiveSurvivorSpace(x) (((asMemorySpace(x)->spaceType==SURVIVOR_SPACE_1) || (asMemorySpace(x)->spaceType==SURVIVOR_SPACE_2)) && isCurrentSpace(x))
#define markSpaceAsCurrent(x) do {asMemorySpace(x)->spaceFlags |= SPACE_IS_CURRENT_SPACE; } while (0)
#define markSpaceAsNotCurrent(x) do {asMemorySpace(x)->spaceFlags &= ~SPACE_IS_CURRENT_SPACE; } while (0)

#define isStackSpace(x) (asMemorySpace(x)->spaceType==STACK_SPACE)
#define isPointerSpace(x) ((asMemorySpace(x)->spaceFlags & SPACE_IS_POINTER_SPACE) == SPACE_IS_POINTER_SPACE)

#define isTopHeaderSpace(x) (isStackSpace(x) || isPointerSpace(x))
#define makeTopHeaderSpace(x) ((asMemorySpace(x))->spaceFlags |= SPACE_HAS_TOP_HEADERS)

#define spaceIsScavenged(x) (((x)==EdenSpace) || ((x)==SurvivorSpace1) || ((x)==SurvivorSpace2))
#define makeScavengedSpace(x) ((asMemorySpace(x))->spaceFlags |= SPACE_IS_SCAVENGED)

#define spaceIsStackManaged(x) isStackSpace(x)
#define makeStackManagedSpace(x) ((asMemorySpace(x))->spaceFlags |= SPACE_IS_STACK_MANAGED)

#define spaceIsMarkSweepManaged(x) ((x)==OldSpace)
#define makeMarkSweepManagedSpace(x) ((asMemorySpace(x))->spaceFlags |= SPACE_IS_MARK_SWEEP_MANAGED)

#define spaceHasSpaceObject(x) (((asMemorySpace(x))->spaceFlags & SPACE_HAS_SPACE_OBJECT) == SPACE_HAS_SPACE_OBJECT)
#define markHasSpaceObject(x) ((asMemorySpace(x))->spaceFlags |= SPACE_HAS_SPACE_OBJECT)

typedef struct {
  oop frame;
  oop stackOffset;
  oop pcOffset;
  oop method;
  oop methodContext;
  oop contextId;
  oop stackBody[];
} contextStruct;
#define asContext(x) ((contextStruct *)objectBody(x))

typedef struct {
	oop *stackPointer;
	oop *stackOffsetPointer;
	oop *localsPointer;
	uint8_t *currentPCPointer;
	oop *currentPCOffset;
	uint64_t *stackSpaceFirstFreeBlock;
	contextStruct *currentContextBody;
} fastContextStruct;

extern oop contextCopy(oop context);

typedef struct {
  uint64_t size;
  uint16_t flags;
#define BYTES 1
#define INDEXED 2
#define RELOCATED 4
#define FREE 8
#define MARK 16
#define QUEUED_FOR_MARK 32
#define SPACE_OBJECT 64
#define VM_MIGRATION_NEW 128
  uint16_t flips;
  uint32_t numberOfNamedInstanceVariables;
  oop stClass;
  oop identityHash;
  oop bodyPointer;
} objectHeaderStruct;

#define asObjectHeader(x) ((objectHeaderStruct *)oopPtr(x))
#define objectBody(x) (asObjectHeader(x)->bodyPointer)
#define queueForMarkObject(x) do {asObjectHeader(x)->flags |= QUEUED_FOR_MARK;} while (0)
#define unqueueForMarkObject(x) do {asObjectHeader(x)->flags &= ~QUEUED_FOR_MARK;} while (0)
#define markObject(x) do {asObjectHeader(x)->flags |= MARK;} while (0)
#define unmarkObject(x) do {asObjectHeader(x)->flags &= ~MARK;} while (0)
#define markObjectFree(x) do {asObjectHeader(x)->flags |= FREE;} while (0)
#define unmarkObjectFree(x) do {asObjectHeader(x)->flags &= ~FREE;} while (0)
#define markObjectRelocated(x) do {asObjectHeader(x)->flags |= RELOCATED;} while (0)
#define unmarkObjectRelocated(x) do {asObjectHeader(x)->flags &= ~RELOCATED;} while (0)
#define bodyHeaderPointer(x) (*((uint64_t *)objectBody(x) + totalObjectSize(x)))
#define setBodyHeaderPointer(x) if ((asObjectHeader(x))->bodyPointer != 0) \
	(*((uint64_t *)objectBody(x) + totalObjectSize(x)) = (x))
#define markSpaceObject(x) do {asObjectHeader(x)->flags |= SPACE_OBJECT;} while (0)
#define isSpaceObject(x) ((asObjectHeader(x)->flags & SPACE_OBJECT) == SPACE_OBJECT)
#define isVMMigrationNew(x) ((asObjectHeader(x)->flags & VM_MIGRATION_NEW) == VM_MIGRATION_NEW)
#define markVMMigrationNew(x) do {asObjectHeader(x)->flags |= VM_MIGRATION_NEW;} while (0)
#define unmarkVMMigrationNew(x) do {asObjectHeader(x)->flags &= ~VM_MIGRATION_NEW;} while (0)

typedef struct {
  oop bytecodes;
  oop numberOfArguments;
  oop numberOfTemporaries;
  oop localVariableNames;
  oop stackNeeded;
  oop polymorphicInlineCache;
  oop sourceOffsets;
  oop mclass;
  oop selector;
  oop kit;
  oop literals[];
} compiledMethodStruct;


// This definition is used when we need to change the shape of the compiled method.
// Since the VM knows and depends on the shape of the compiled method, we must proceed in several stages.
// Note that CompiledBlock must be the same size as CompiledMethod so the VM can treat them interchangeably and find the
// literals field at the same offset in the structure.
//
//   1) Put the new definition in the commented structure below
//   2) Change getLiteral to the commented version below
//   3) Compile the new VM.  This VM will check a flag in the compiled method to tell which definition to use
//   4) In the image, copy CompiledMethod to CompiledMethod2 changing the definition as required (copy over all methods)
//   5) In the image, copy CompiledBlock to CompiledBlock2 changing the definition as required (copy over all methods)
//	 6) In CodeGenerator >> method, change CompiledMethod to CompiledMethod2
//   7) Cascade in a call to markVMMigrationNew
//	 8) Do the same for all (both) references to CompiledBlock - change to CompiledBlock2 and cascade markVMMigrationNew
//   9) Fileout all sources
//   10) Call CodeContext setupBytecodeBlocks
//        (this method clears the global cache containing blocks which will hold onto old compiled methods)
//   11) Filein all sources
//   12) Global garbage collect
//   13) Save the image and re-launch
//   14) Global garbage collect
//   15) Check that CompiledMethod allInstances is empty
//   16) Check that CompiledBlock allInstances is empty
//   17) Save the image and exit
//   18) Change getLiteral below to the original version and recompile the VM
//   19) Relaunch the image.
//   20) Change CompiledMethod to have the same structure as CompiledMethod2
//   21) Change CompiledBlock to have the same structure as CompiledBlock2
//   22) Change CodeGenerator >> method back to the original version using CompiledMethod and removing markVMMigrationNew
//   23) Change all references to CompiledBlock2 back to the original version using CompiledBlock and removing markVMMigrationNew
//   24) Fileout all sources
//   25) Call CodeContext setupBytecodeBlocks
//   26) File in all sources
//   27) Global garbage collect
//   28) Save the image and re-launch (Some calls in the current stack frame may point to old compiled methods)
//   29) Global garbage collect
//   30) Check that CompiledMethod2 allInstances is empty
//   31) Delete CompiledMethod2
//   32) Check that CompiledBlock2 allInstances is empty
//   33) Delete CompiledBlock2
//   34) Save the image
//
//  Welcome to the club of self-brain surgeons


// typedef struct {
//   oop bytecodes;
//   oop numberOfArguments;
//   oop numberOfTemporaries;
//   oop localVariableNames;
//   oop stackNeeded;
//   oop polymorphicInlineCache;
//   oop sourceOffsets;
//   oop mclass;
//   oop selector;
//   oop kit;
//   oop literals[];
// } compiledMethodStruct2;

#define asCompiledMethod(x) ((compiledMethodStruct *) objectBody(x))
// #define asCompiledMethod2(x) ((compiledMethodStruct2 *) objectBody(x))
#define getLiteral(x,y) (asCompiledMethod(x)->literals[y])
//#define getLiteral(x,y) (isVMMigrationNew(x)?asCompiledMethod2(x)->literals[y]:asCompiledMethod(x)->literals[y])
#define getBytecodes(x) (asCompiledMethod(x)->bytecodes)
#define PIC_CACHE_SIZE 4



typedef struct {
	oop picClass;
	oop picMethod;
} picCacheEntryStruct;

typedef struct {
	uint64_t bytecodeOffset;
	picCacheEntryStruct entry[PIC_CACHE_SIZE];
} picCacheStruct;


typedef struct {
  oop method;
  oop methodContext;
  oop copiedValues;
} blockClosureStruct;
#define asBlockClosure(x) ((blockClosureStruct *) objectBody(x))

typedef struct {
  oop key;
  oop value;
} associationStruct;
#define asAssociation(x) ((associationStruct *)objectBody(x))

typedef struct {
  oop values;
  oop tally;
} dictionaryStruct;
#define asDictionary(x) ((dictionaryStruct *)objectBody(x))

typedef struct {
  oop bytes;
  oop componentSize;
} largeIntegerStruct;

#define asLargeInteger(x) ((largeIntegerStruct *)objectBody(x))
#define largeComponentSize(x) stIntToC(asLargeInteger(x)->componentSize)

typedef struct {
  oop values;
  oop tally;
} identityDictionaryStruct;
#define asIdentityDictionary(x) ((identityDictionaryStruct *)objectBody(x))

typedef struct {
  oop superclass;
  oop methodDictionary;
  oop flags;
#define BEHAVIOR_BYTES 1
#define BEHAVIOR_INDEXED 2
  oop subclasses;
  oop instVarNames;
} behaviorStruct;
#define asBehavior(x) ((behaviorStruct *)objectBody(x))
#define Behavior_Flags(p) (stIntToC(asBehavior(p)->flags) & 0xff)
#define Behavior_NumberOfNamedInstVars(p) (stIntToC(asBehavior(p)->flags) >> 16)

typedef struct {
  behaviorStruct behavior;
  oop organization;
  oop name;
  oop kit;
  oop environment;
} classStruct;
#define asClass(x) ((classStruct *)objectBody(x))

typedef struct {
  behaviorStruct behavior;
  oop organization;
  oop thisClass;
} metaclassStruct;
#define asMetaclass(x) ((metaclassStruct *)objectBody(x))

typedef struct {
	classStruct classHeader;
	oop current;
	oop imageName;
	oop sourceFiles;
	oop sourceFileNames;
	oop specialSelectors;
} systemClassStruct;
#define asSystemClass(x) ((systemClassStruct *)objectBody(x))

typedef struct {
  uint64_t value;
} osHandleStruct;
#define asOsHandle(x) ((osHandleStruct *)objectBody(x))
#define osHandleValue(x) (asOsHandle(x)->value)

// This structure overlays an UninterpretedBytes object
// It serves as the handle value in a Socket object
typedef struct {
  uint64_t socketHandle;
  socklen_t addressLength;
  struct sockaddr_in address;
} socketStruct;
#define asSocket(x) ((socketStruct *)objectBody(x))

typedef struct {
  oop event;
  oop context;
  oop application;
  oop wakeupTime;
  oop wakeupSemaphore;
  oop idleSemaphore;
  oop inAppTransactionFiler;
  oop running;
  oop eventQueue;
} systemStruct;
#define asSystem(x) ((systemStruct *)objectBody(x))

typedef struct {
  oop position;
  oop readLimit;
  oop collection;
  oop handle;
  oop isBinary;
} fileStreamStruct;
#define asFileStream(x) ((fileStreamStruct *)objectBody(x))

typedef struct {
  oop x;
  oop y;
} pointStruct;
#define asPoint(x) ((pointStruct *)objectBody(x))

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t development;
  uint64_t length;
} imageHeaderStruct;
#define asImageHeader(x) ((imageHeaderStruct *)oopPtr(x))

typedef struct {
  uint8_t *bytes;
} imageStruct;
#define asImage(x) ((imageStruct *)objectBody(x))

typedef struct{
	oop message;
	oop action;
	oop result;
} exceptionStruct;
#define asException(x) ((exceptionStruct *)objectBody(x))

#define nextBytecode() ((*fastContext.currentPCOffset)+=SMALLINTEGER_INCREMENT(1) , *fastContext.currentPCPointer++)
#define previousBytecode() ((*fastContext.currentPCOffset)-=SMALLINTEGER_INCREMENT(1) , *fastContext.currentPCPointer--)
#define peekBytecodeOffset(o) (fastContext.currentPCPointer[(o)])
#define peekBytecode() peekBytecodeOffset(0)
#define peekBytecode1() peekBytecodeOffset(1)
#define peekBytecode2() peekBytecodeOffset(2)
#define peekBytecode3() peekBytecodeOffset(3)
#define peekBytecode4() peekBytecodeOffset(4)
#define peekBytecode5() peekBytecodeOffset(5)
// #define bytecodePCToOffset(c) (asOop(((c)->pc) - asObjectHeader(asCompiledMethod((c)->method)->bytecodes)->bodyPointer))
// #define bytecodeOffsetToPC(c,o) ((o) + asObjectHeader(asCompiledMethod((c)->method)->bytecodes)->bodyPointer)


#define jump(offset) do {(*fastContext.currentPCOffset) += (SMALLINTEGER_INCREMENT(1) * (offset)), fastContext.currentPCPointer += (offset);} while(0)
#define previousFrame(c) asContext((asContext(c)->frame))
#define stackAt(i) (fastContext.localsPointer[i])
#define getReceiver() (stackAt(0))

//#define stackForFrameAt(c,i) (*(oopPtr(previousFrame((c))->stack + (i)*sizeof(oop))))
#define stackForFrameAt(c,i) (previousFrame(c)->stackBody[(i)])
#define getReceiverForFrame(c) (stackForFrameAt((c),0))

#define getLocal(x)  (stackAt(1+(x)))
#define setLocal(x,y)  (stackAt(1+(x))=(y))

#define push(x) do {(*fastContext.stackSpaceFirstFreeBlock)++; asObjectHeader(currentContext)->size += sizeof(oop); *fastContext.stackPointer++ = (x); (*fastContext.stackOffsetPointer)+=SMALLINTEGER_INCREMENT(1);} while (0)
#define pop() ((*fastContext.stackSpaceFirstFreeBlock)--, asObjectHeader(currentContext)->size -= sizeof(oop), (*fastContext.stackOffsetPointer)-=SMALLINTEGER_INCREMENT(1) , *(--fastContext.stackPointer) )
#define top() (*(fastContext.stackPointer - 1))

#define simlog(...) do {logPtr += sprintf (logPtr, __VA_ARGS__);} while(0)
#define simlogStack(context,operation) do {simlog2(operation " SP: %lx TOS: 0x%lx\n", (unsigned long) (context)->stack, *((unsigned long *) ((context)->stack+1)));} while(0)

// Immediate objects

#define IMMEDIATE_TAG_MASK 7
#define IMMEDIATE_SHIFT 3
#define SMALLINTEGER_INCREMENT(x) (x << IMMEDIATE_SHIFT)
#define INT_TAG 1
#define CHAR_TAG 2
#define FLOAT_TAG 3
#define CONTEXT_POINTER_TAG 6

#define isImmediate(x)  ((((uint64_t)(x)) & IMMEDIATE_TAG_MASK) != 0)
#define isCharacter(x)  ((((uint64_t)(x)) & IMMEDIATE_TAG_MASK) == CHAR_TAG)
#define isSmallInteger(x) ((((uint64_t)(x)) & IMMEDIATE_TAG_MASK) == INT_TAG)
#define isFloat(x) ((((uint64_t)(x)) & IMMEDIATE_TAG_MASK) == FLOAT_TAG)

#define stIntToC(x) (((int64_t)(x))>>IMMEDIATE_SHIFT)
#define cIntToST(x) ((oop)(((uint64_t)(x))<<IMMEDIATE_SHIFT | INT_TAG))

#define cCharToST(x) ((oop)(((uint64_t)(x))<<IMMEDIATE_SHIFT | CHAR_TAG))
extern oop asSumLargeInteger(int64_t x);

// ST floats are in the format:
// sign: 1 bit    exponent: 8 bits  mantissa 52 bits   tag: 3 bits
//
// IEEE floats are in the format:
// sign: 1 bit    exponent: 11 bits  mantissa 52 bits
//

// To convert an ST float to an IEEE float, shift left by 1 then convert to a signed integer so the exponent
// is sign extended when we shift back.
typedef union {
	uint64_t intValue;
	double doubleValue;
	struct {
		uint64_t mantissa:52;
		uint64_t exponent: 11;
		uint64_t sign: 1;
	} structValue;
} doubleConverter;
extern doubleConverter dc;

#define stFloatToC(x) (\
	(x)==FLOAT_TAG?0.0:(\
	dc.structValue.mantissa = ((x)&0x007FFFFFFFFFFFF8ull)>>3,\
	dc.structValue.exponent = ((((x)&0x7F80000000000000ull)>>55)+1023-127),\
	dc.structValue.sign=(((x)&0x8000000000000000ull)>>63),\
	dc.doubleValue))
	
#define cFloatToST(x) (\
	dc.doubleValue = (x),\
	(dc.doubleValue==0.0)?FLOAT_TAG:(\
		(((uint64_t)dc.structValue.mantissa)<<3) | \
		((((uint64_t)dc.structValue.exponent)-1023 + 127) << 55) | \
		(((uint64_t)dc.structValue.sign) << 63) | FLOAT_TAG))


// Object header flags
#define isBytes(x) ((asObjectHeader(x)->flags & BYTES) == BYTES)
#define isMarked(x) ((asObjectHeader(x)->flags & MARK) == MARK)
#define isQueuedForMark(x) ((asObjectHeader(x)->flags & QUEUED_FOR_MARK) == QUEUED_FOR_MARK)
#define isFree(x) ((!isImmediate(x)) && ((asObjectHeader(x)->flags & FREE) == FREE))
#define isContextPointer(x) ((((uint64_t)(x)) & IMMEDIATE_TAG_MASK) == CONTEXT_POINTER_TAG)
#define stripTags(x) (((uint64_t)(x)) & ~IMMEDIATE_TAG_MASK)
#define markAsContextPointer(x) (((uint64_t)(x)) | CONTEXT_POINTER_TAG)
#define markAsSmallInteger(x) (((uint64_t)(x)) | INT_TAG)
#define isRelocated(o) ((asObjectHeader(o)->flags & RELOCATED) != 0)

// Class testing

#define isUninterpretedBytes(x) (!isImmediate(x) && (asObjectHeader(x)->stClass == ST_UNINTERPRETED_BYTES_CLASS))
#define isArray(x) (!isImmediate(x) && (asObjectHeader(x)->stClass == ST_ARRAY_CLASS))
#define isOSHandle(x) (!isImmediate(x) && (asObjectHeader(x)->stClass == ST_OS_HANDLE_CLASS))
#define isNil(x) ((x) == ST_NIL)
#define notNil(x) ((x) != ST_NIL)
#define isTrue(x) ((x) == ST_TRUE?1:0)
#define isFalse(x) ((x) == ST_FALSE?1:0)
#define classOf(x) ((isSmallInteger(x))? ST_SMALL_INTEGER_CLASS :\
		(isCharacter(x)) ? ST_CHARACTER_CLASS :\
		(isFloat(x)) ? ST_FLOAT_CLASS : \
		(isContextPointer(x)) ? ST_SMALL_INTEGER_CLASS : \
		asObjectHeader(x)->stClass)

#define isLargePositiveInteger(x) ((classOf(x)==ST_LARGE_POSITIVE_INTEGER_CLASS))
#define isLargeNegativeInteger(x) (classOf(x)==ST_LARGE_NEGATIVE_INTEGER_CLASS)
#define isLargeInteger(x) (isLargePositiveInteger(x) ||isLargeNegativeInteger(x))

#define stOffsetToPC(x) ((x)==asOop(NULL)?asOop(NULL):(asOop(&((uint8_t *)Spaces[(((x)>> 56) - 1) & 0xFF]->space) [((x) & 0xFFFFFFFFFFFFF8) >> 3])))

// Macros to read and write instance variables.
// If you write an instance variable with an oop that points to new space,
// you may need to register the object in the remembered set so that the
// scavenging garbage collector can find it.

#define registerIfNeeded(object,value)     do {if(!isObjectInAnyNewSpace(object) && (isObjectInAnyNewSpace(value)))	\
      registerRememberedSetObject(asOop(object)); } while (0)

//#define basicInstVarAtIntPut(object, index, value) (oopPtr(asObjectHeader(object)->bodyPointer))[index] = (value)

#define basicInstVarAtIntPut(object, index, value) do { if ((index) >= totalObjectSize(object)) { \
		raiseSTError (ST_ERROR_CLASS, "Index out of bounds");		} \
		(oopPtr(asObjectHeader(object)->bodyPointer))[index] = (value); } while (0)

	
#define instVarAtInt(object, index) ((oopPtr(asObjectHeader(object)->bodyPointer))[index])
#define instVarAtIntPut(object,index,value) do { \
		registerIfNeeded(object, value); \
		basicInstVarAtIntPut(object, index, value);} while(0)

#define instVarAt(object,index) (instVarAtInt(object, stIntToC(index)))
#define instVarAtPut(object,index,value) (instVarAtIntPut(object, stIntToC(index), value)

#define indexedVarAtInt(object,index) (instVarAtInt(object, asObjectHeader(object)->numberOfNamedInstanceVariables + (index) - 1))
#define indexedVarAtIntPut(object,index,value) instVarAtIntPut(object, asObjectHeader(object)->numberOfNamedInstanceVariables + (index) - 1, value)

#define indexedVarAt(object,index) indexVarAtInt(object, stIntToC(index))
#define indexedVarAtPut(object,index,value) indexedVarAtIntPut(object, stIntToC(index), value)

#define basicByteAtInt(object,index) (((uint8_t *)asObjectHeader(object)->bodyPointer)[(index) - 1])
#define basicByteAtIntPut(object,index,value) do {((uint8_t *)asObjectHeader(object)->bodyPointer)[(index) - 1] = (value);} while(0)

// Object Size macros

#define objectHeaderSize() (sizeof(objectHeaderStruct))
#define objectHeaderOopSize() (objectHeaderSize() / sizeof(oop))
#define memorySize(object) (asObjectHeader(object)->size)
#define basicByteSize(object) (memorySize(object) - objectHeaderSize())
#define totalObjectSize(object) ((basicByteSize(object) + sizeof(oop) - 1) / sizeof(oop))
#define indexedObjectSize(object) (totalObjectSize(object) - asObjectHeader(object)->numberOfNamedInstanceVariables)

// Special Selectors
// Special selectors are symbols which the VM knows about and needs to use.  These are stored in the SimTalkSystem class
// in a class instance variable.  The variable contains an array of two elements for each selector - the pointer to the
// symbol itself and an integer representing the number of arguments for that symbol.

#define specialSelectors(i) asOop(instVarAtInt(asSystemClass(ST_SYSTEM_CLASS)->specialSelectors, (i)*2))
#define specialSelectorArguments(i) stIntToC(instVarAtInt(asSystemClass(ST_SYSTEM_CLASS)->specialSelectors, (i)*2 + 1))

#define SPECIAL_PLUS 0x00
#define SPECIAL_MINUS 0x01
#define SPECIAL_TIMES 0x02
#define SPECIAL_NOT 0x03
#define SPECIAL_IDENTICAL 0x04
#define SPECIAL_NOT_IDENTICAL 0x05
#define SPECIAL_EQUALS 0x06
#define SPECIAL_NOT_EQUALS 0x07
#define SPECIAL_IS_NIL 0x08
#define SPECIAL_NOT_NIL 0x09
#define SPECIAL_GREATER_THAN 0x0a
#define SPECIAL_LESS_THAN 0x0b
#define SPECIAL_GREATER_THAN_OR_EQUAL 0x0c
#define SPECIAL_LESS_THAN_OR_EQUAL 0x0d
#define SPECIAL_EVALUATE 0x0e
#define SPECIAL_PRINT_STRING 0x0f
#define SPECIAL_RAISE_SIGNAL 0x10
#define SPECIAL_PERFORM_WITH_ARGUMENTS 0x11
#define SPECIAL_HALT 0x12
#define SPECIAL_DEBUGIT 0x13
#define SPECIAL_EVALUATE_JSON 0x14

#define ERROR_EXIT exit(1)


#define PRIMITIVE_FAIL(x) do {push (cIntToST(x)); push (receiver); return;} while(0)

#define DEFINE_LOCALS int __localVarNumber = 0; oop volatile *__localBase = fastContext.stackPointer
#define DEFINE_LOCAL(x) oop x ## _localVarIndex = __localVarNumber++; push(ST_NIL)
#define GET_LOCAL(x) __localBase[x ## _localVarIndex]
#define SET_LOCAL(x, y) do {oop __resultOop = (y); __localBase[x ## _localVarIndex] = __resultOop;} while (0)
#define FREE_LOCALS do {while (__localVarNumber-- > 0) {pop();}} while (0)


// Externs for VM global variables
extern char *imageFilename;
extern int webSocketPortNumber;
extern int debugWebSocketPort;
extern char *logPtr, logString[];
extern oop *stack;
extern oop currentContext, stopFrame;
extern fastContextStruct fastContext;
extern memorySpaceStruct *currentStackSpace;
extern int breakpointHit;
extern int suspended;
extern char errorString[];
extern int eventWaitingFlag;

typedef void (*cleanupProcType)(void);
extern cleanupProcType cleanupProcs[];
extern int cleanupProcIndex;

extern void captureFastContext(oop context);
extern void invoke(oop method, uint64_t numArgs);
extern void invokeBlock(oop blockClosure, uint64_t numArgs);
extern oop findCompiledMethod (oop selector, oop aClass);
extern void dispatch (oop selector, uint64_t numArgs);
extern void setupInterpreter(memorySpaceStruct *space);
extern void interpret(void);
extern int basicInterpret(int maxBytecodes);
extern void invokePrimitive(unsigned short primitiveNumber);
extern void initializePrimitiveTable(void);
extern void launchImage(void);

extern void allocateImageSpace (uint64_t size);
extern void allocateImageRememberedSet (uint64_t size);
extern oop allocateObjectInSpace (uint64_t size, memorySpaceStruct *space);
extern oop newInstanceOfClass (oop behavior, uint64_t indexedVars, memorySpaceStruct *space);
extern void finish(void);
extern oop identityDictionaryAt (oop dictionary, oop key);
extern oop identityDictionaryKeyAtValue (oop dictionary, oop value);
extern oop globalVariableAt(oop symbol);

#define ERROR_BAD_PARAMETER_TYPE 1
typedef void (*primitiveFunction)(void);
extern primitiveFunction primitiveTable[];

extern void initializeFloatPrimitives(void);
extern void initializeIntegerPrimitives(void);
extern void initializeSocketPrimitives(void);
extern void initializeFilePrimitives(void);
extern void initializeMemoryPrimitives();

extern void handleSocketCommands(void);

extern void registerRememberedSetObject(oop);
extern int unregisterRememberedSetObject(oop);

extern uint8_t *readResource (char *resourceName, uint32_t *sizePtr);

extern void setupRemoteSocket(void);
typedef uint64_t readFunctionType(uint8_t *buffer, uint64_t size, void *data);
extern uint64_t loadImage(readFunctionType *readFunction, void *data, char *filename);
extern void saveImage(FILE *file);
extern memorySpaceStruct *allocateSpace (uint64_t size);
extern void reallocateSpace(int spaceIndex, uint64_t size);
extern uint64_t oopToOffset (oop p);
extern void terminateSocket(void);

extern int STStringToC(oop string, char* cstring);

extern int stStringCollectionToC(oop stringCollection, char ***cArray);
extern void freeStringCollection(char **cArray);
extern oop CStringToST(char *cString);

extern int64_t getTimeNsec(void);
extern int startSymbolLog (uint64_t method);
extern void endSymbolLog (uint64_t method);
extern void dumpWalkback(char *message);
extern void dispatchSpecial0 (unsigned int selectorNumber, oop receiver);
extern void dispatchSpecial1 (unsigned int selectorNumber, oop receiver, oop arg1);
extern void dispatchSpecial2 (unsigned int selectorNumber, oop receiver, oop arg1, oop arg2);
extern void raiseSTError(oop errorClass, char *message);
extern void scavenge();
extern uint64_t nextObjectIncrement(oop object);
extern oop stPtrToC(oop x);
extern int checkObject(oop x);
extern void auditImage();
extern void logBytecode();
extern uint64_t findRememberedSetObject(oop object);
extern void globalGarbageCollect();
extern void startupDebugger();
extern oop smallToLargeInteger(oop x);
extern oop largeIntegerTimes(oop x, oop y);


typedef void (*spaceEnumerationFunction)(memorySpaceStruct *, void *args);
typedef void (*enumerationFunction)(oop, void *args);
typedef void (*pointerEnumerationFunction)(oop *, void *args);

extern void enumerateSpaces(spaceEnumerationFunction function, void *args);
extern void enumerateObjectsInSpace(memorySpaceStruct *space, enumerationFunction function, void *args);
extern void enumeratePointersInSpace(memorySpaceStruct *space, pointerEnumerationFunction function, void *args);

extern memorySpaceStruct *Spaces[];
extern memorySpaceStruct *EdenSpace;
extern memorySpaceStruct *SurvivorSpace1;
extern memorySpaceStruct *SurvivorSpace2;
extern memorySpaceStruct *RememberedSet;
extern memorySpaceStruct *OldSpace;
extern memorySpaceStruct *WellKnownObjects;
extern memorySpaceStruct *StackSpace;

extern memorySpaceStruct *ActiveSurvivorSpace;
extern memorySpaceStruct *InactiveSurvivorSpace;
extern int tracing;

// Well known objects
//
// Well known objects are objects that the VM needs to refer to.  They are stored in their own
// memory space and are referenced by integer indices starting at 0.

#define O_NIL 0
#define O_TRUE 1
#define O_FALSE 2
#define O_SYSTEM_DICTIONARY 3
#define O_SYMBOL_TABLE 4
#define O_START_OBJECT 5
#define O_START_SELECTOR 6
#define O_START_CONTEXT 7
#define O_SMALL_INTEGER_CLASS 8
#define O_CHARACTER_CLASS 9
#define O_BLOCK_CLOSURE_CLASS 10
#define O_ARRAY_CLASS 11
#define O_FLOAT_CLASS 12
#define O_OBSOLETE_CLASS 13
#define O_LARGE_POSITIVE_INTEGER_CLASS 14
#define O_LARGE_NEGATIVE_INTEGER_CLASS 15
#define O_OS_HANDLE_CLASS 16
#define O_BYTE_STRING_CLASS 17
#define O_BYTE_SYMBOL_CLASS 18
#define O_UNINTERPRETED_BYTES_CLASS 19
#define O_SYSTEM_CLASS 20
#define O_CLASS_CLASS 21
#define O_METACLASS_CLASS 22
#define O_COMPILED_BLOCK_CLASS 23
#define O_ASSOCIATION_CLASS 24
#define O_CODE_CONTEXT_CLASS 25
#define O_BYTE_ARRAY_CLASS 26
#define O_BYTECODE_TABLE 27
#define O_SMALLTALK_PARSER_CLASS 28
#define O_EXCEPTION_HANDLERS 29
#define O_MESSAGE_NOT_UNDERSTOOD_CLASS 30
#define O_ERROR_CLASS 31
#define O_JSON_PARSER_CLASS 32
#define O_MEMORY_SPACE_CLASS 33
#define O_LAST_WELL_KNOWN_OBJECT 33

#define ST_NIL ((oop)(WellKnownObjects->space[O_NIL]))
#define ST_TRUE ((oop)(WellKnownObjects->space[O_TRUE]))
#define ST_FALSE ((oop)(WellKnownObjects->space[O_FALSE]))
#define ST_SYSTEM_DICTIONARY ((oop)(WellKnownObjects->space[O_SYSTEM_DICTIONARY]))
#define ST_SYMBOL_TABLE ((oop)(WellKnownObjects->space[O_SYMBOL_TABLE]))
#define ST_START_OBJECT ((oop)(WellKnownObjects->space[O_START_OBJECT]))
#define ST_START_SELECTOR ((oop)(WellKnownObjects->space[O_START_SELECTOR]))
#define ST_START_CONTEXT ((oop)(WellKnownObjects->space[O_START_CONTEXT]))
#define ST_SMALL_INTEGER_CLASS ((oop)(WellKnownObjects->space[O_SMALL_INTEGER_CLASS]))
#define ST_CHARACTER_CLASS ((oop)(WellKnownObjects->space[O_CHARACTER_CLASS]))
#define ST_BLOCK_CLOSURE_CLASS ((oop)(WellKnownObjects->space[O_BLOCK_CLOSURE_CLASS]))
#define ST_ARRAY_CLASS ((oop)(WellKnownObjects->space[O_ARRAY_CLASS]))
#define ST_FLOAT_CLASS ((oop)(WellKnownObjects->space[O_FLOAT_CLASS]))
#define ST_OBSOLETE_CLASS ((oop)(WellKnownObjects->space[O_OBSOLETE_CLASS]))
#define ST_LARGE_POSITIVE_INTEGER_CLASS ((oop)(WellKnownObjects->space[O_LARGE_POSITIVE_INTEGER_CLASS]))
#define ST_LARGE_NEGATIVE_INTEGER_CLASS ((oop)(WellKnownObjects->space[O_LARGE_NEGATIVE_INTEGER_CLASS]))
#define ST_OS_HANDLE_CLASS ((oop)(WellKnownObjects->space[O_OS_HANDLE_CLASS]))
#define ST_BYTE_STRING_CLASS ((oop)(WellKnownObjects->space[O_BYTE_STRING_CLASS]))
#define ST_BYTE_SYMBOL_CLASS ((oop)(WellKnownObjects->space[O_BYTE_SYMBOL_CLASS]))
#define ST_UNINTERPRETED_BYTES_CLASS ((oop)(WellKnownObjects->space[O_UNINTERPRETED_BYTES_CLASS]))
#define ST_SYSTEM_CLASS ((oop)(WellKnownObjects->space[O_SYSTEM_CLASS]))
#define ST_CLASS_CLASS ((oop)(WellKnownObjects->space[O_CLASS_CLASS]))
#define ST_METACLASS_CLASS ((oop)(WellKnownObjects->space[O_METACLASS_CLASS]))
#define ST_COMPILED_BLOCK_CLASS ((oop)(WellKnownObjects->space[O_COMPILED_BLOCK_CLASS]))
#define ST_ASSOCIATION_CLASS ((oop)(WellKnownObjects->space[O_ASSOCIATION_CLASS]))
#define ST_CODE_CONTEXT_CLASS ((oop)(WellKnownObjects->space[O_CODE_CONTEXT_CLASS]))
#define ST_BYTE_ARRAY_CLASS ((oop)(WellKnownObjects->space[O_BYTE_ARRAY_CLASS]))
#define ST_BYTECODE_TABLE ((oop)(WellKnownObjects->space[O_BYTECODE_TABLE]))
#define ST_SMALLTALK_PARSER_CLASS ((oop)(WellKnownObjects->space[O_SMALLTALK_PARSER_CLASS]))
#define ST_EXCEPTION_HANDLERS ((oop)(WellKnownObjects->space[O_EXCEPTION_HANDLERS]))
#define ST_MESSAGE_NOT_UNDERSTOOD_CLASS ((oop)(WellKnownObjects->space[O_MESSAGE_NOT_UNDERSTOOD_CLASS]))
#define ST_ERROR_CLASS ((oop)(WellKnownObjects->space[O_ERROR_CLASS]))
#define ST_JSON_PARSER_CLASS ((oop)(WellKnownObjects->space[O_JSON_PARSER_CLASS]))
#define ST_MEMORY_SPACE_CLASS ((oop)(WellKnownObjects->space[O_MEMORY_SPACE_CLASS]))


// Space testing macros
//
// These macros determining which memory space contains the object

#define isObjectInSpace(o,s) ((!isImmediate(o)) && (isStackSpace(s)? ((asOop(o) >= asOop(&s->space[s->lastFreeBlock+1])) && (asOop(o) < asOop(&s->space[s->spaceSize / sizeof(oop)]))) : ((asOop(o) >= asOop(&s->space[0])) && (asOop(o) < asOop(&s->space[s->firstFreeBlock])))))
#define isObjectInActiveSurvivorSpace(o) (isObjectInSpace((o),ActiveSurvivorSpace))
#define isObjectInInactiveSurvivorSpace(o) (isObjectInSpace((o),InactiveSurvivorSpace))
#define isObjectInEdenSpace(o) (isObjectInSpace((o),EdenSpace))
#define isObjectInNewSpace(o) (isObjectInSpace((o),EdenSpace) || isObjectInSpace((o),ActiveSurvivorSpace))
#define isObjectInAnyNewSpace(o) (isObjectInNewSpace((o)) || isObjectInActiveSurvivorSpace((o)))
#define isObjectInOldSpace(o) (isObjectInSpace((o),OldSpace))
#define isObjectInObjectSpace(o) (isObjectInNewSpace(o) || (isObjectInOldSpace(o)))
#define isObjectInStackSpace(o) (isObjectInSpace((o),StackSpace))
#define isObjectInWellKnownObjectsSpace(o) (isObjectInSpace((o),WellKnownObjectsSpace))
#define isObjectInActiveMemorySpace(o) (isObjectInNewSpace(o) ||  isObjectInOldSpace(o))

#define isBodyInSpace(o,s) ((!isImmediate(o)) && ((asObjectHeader(o)->bodyPointer) >= (oop)&((s)->space[s->lastFreeBlock + 1])) && ((asObjectHeader(o)->bodyPointer) < (oop) endOfSpace(s)))
#define isBodyInEdenSpace(o) (isBodyInSpace((o),EdenSpace))
#define isBodyInActiveSurvivorSpace(o) (isBodyInSpace((o),ActiveSurvivorSpace))
#define isBodyInInactiveSurvivorSpace(o) (isBodyInSpace((o),InactiveSurvivorSpace))
#define isBodyInOldSpace(o) (isBodyInSpace((o),OldSpace))
#define isValidPointerOop(o) (isObjectInNewSpace(o) || isObjectInObjectSpace(o) || isObjectInStackSpace(o))
#define isValidOop(o) (isImmediate(o) || isSpaceObject(o) || isValidPointerOop(o))

#define WARN_STACK_THRESHOLD 2048
#define CRITICAL_STACK_THRESHOLD 1024


extern uint64_t wakeupTime;
extern unsigned long Development;
extern char walkbackDump[];

#pragma clang diagnostic pop