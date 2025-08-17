// file_primitives.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT


#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-sizeof-expression"
//
// Created by maxim on 2017-06-13.
//
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "object.h"

#define PRIM_EXISTS 1403
#define PRIM_OPEN 1500
#define PRIM_OPEN_MODE 1501
#define PRIM_CLOSE 1502
#define PRIM_NEXT 1503
#define PRIM_NEXT_COLON 1504
#define PRIM_NEXT_PUT 1505
#define PRIM_NEXT_PUT_ALL 1506
#define PRIM_SKIP 1507
#define PRIM_UPTOEND 1508
#define PRIM_AT_END 1509
#define PRIM_FLUSH 1510
#define PRIM_POSITION 1511
#define PRIM_POSITION_COLON 1512
#define PRIM_PATH_DELIMITER 1513

// Returns an OS handle to the file pointer such that the pointer can be retrieved for work on this file
// defaults to r+
void primOpen(){
	DEFINE_LOCALS;
	DEFINE_LOCAL(filestream);
	DEFINE_LOCAL(path);
	
	SET_LOCAL(filestream, getReceiver());
	SET_LOCAL(path, getLocal(0));

    char *filePath = malloc((unsigned long) basicByteSize(GET_LOCAL(path) + 1));
	if(STStringToC(GET_LOCAL(path), filePath)){
		FREE_LOCALS;
		free(filePath);
		push (cIntToST(1));
		push (cIntToST(0));
		return;
    }

    FILE *writable;
	if((writable = fopen(filePath, "r+")) == (FILE *) EOF){
        LOGE("(primOpen) ERROR: could not open requested file");
		FREE_LOCALS;
        free(filePath);
        push (cIntToST(1));
        push (cIntToST(errno));
        return;
    }

//    LOGW("(primOpen) File opened successfully. {%s} {fp: %"PRIx64"}", filePath, writable);
    free(filePath);

    oop handleOop = newInstanceOfClass(ST_OS_HANDLE_CLASS, sizeof(writable), EdenSpace);
    asOsHandle(handleOop)->value = asOop(writable);

    asFileStream(GET_LOCAL(filestream))->handle = handleOop;
	oop fileStream = GET_LOCAL(filestream);

	FREE_LOCALS;

	push (cIntToST(0));
    push (fileStream);
}

// Returns an OS handle to the file pointer such that the pointer can be retrieved to work on this file
// Opens the file with the mode specified
void primOpenMode(){
	
	DEFINE_LOCALS;
	DEFINE_LOCAL(filestream);
	DEFINE_LOCAL(path);
	DEFINE_LOCAL(mode);

    SET_LOCAL(filestream, getReceiver());
    SET_LOCAL(path, getLocal( 0));
    SET_LOCAL(mode, getLocal( 1));

    char *filePath = malloc((unsigned long) basicByteSize(GET_LOCAL(path)) + 1);
    if(STStringToC(GET_LOCAL(path), filePath)){
        LOGE("primOpenMode");
        free(filePath);
    }

    char *fileMode = malloc((unsigned long) basicByteSize(GET_LOCAL(mode)) + 1);
    if(STStringToC(GET_LOCAL(mode), fileMode)){
        LOGE("primOpenMode");
		FREE_LOCALS;
        free(fileMode);
        push (cIntToST(1));
        push (cIntToST(errno));
    }

    FILE *file;
    file = fopen(filePath, fileMode);
    if(file == (FILE *) NULL){
		FREE_LOCALS;
        free(filePath);
        free(fileMode);
        push (cIntToST(1));
        push (cIntToST(errno));
        return;
    }
 //   LOGW("(primOpenMode) File opened successfully with permissions: %s . {%s} {fp: %x}", filePath, fileMode, (unsigned int) file);

    free(filePath);
    free(fileMode);

    oop handleOop = newInstanceOfClass(ST_OS_HANDLE_CLASS, sizeof(file), EdenSpace);
    asOsHandle(handleOop)->value = asOop(file);

    asFileStream(GET_LOCAL(filestream))->handle = handleOop;

    push (cIntToST(0));
    push (GET_LOCAL(filestream));
}

// Closes the file
void primClose(){
    oop fileStreamOop = getReceiver();

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    //LOGW("(primClose) Closing file. {fp: %x}", file);
    if(fclose(file) == EOF){
        LOGE("(primClose) ERROR: could not close file");
    }

    push (cIntToST(0));
    push (cIntToST(0));
}

// Returns the next character
void primNext(){
    oop fileStreamOop = getReceiver();
    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    int retrieved = fgetc(file);

    if(retrieved == EOF){
        LOGE("(primNext) ERROR: could not read character from file {fp: %"PRIx64"}", (uint64_t) file);
        if(feof(file)) { LOGE("END OF FILE"); }
        push (cIntToST(1));
        push (cIntToST(errno));
    }

    push (cIntToST(0));
    push (cIntToST(retrieved));
}

// Returns the bytes for the next n characters
void primNextColon(){
    oop fileStreamOop = getReceiver();
    int length = (int) stIntToC(getLocal( 0));
    size_t check;

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

	oop result = newInstanceOfClass (ST_BYTE_ARRAY_CLASS, length, EdenSpace);

    char* buff = malloc(length*sizeof(char));
    check = fread(buff, sizeof(char), length,  file);

    if(check != length){
        LOGE("An error has occurred while reading the file.");
		free(buff);
        push (cIntToST(0));
        push (cIntToST(check));
        return;
    }

    for(int i = 0; i < length; i++){
            //LOGW("Retrieved the following char value: %d", buff[i]);
            basicByteAtIntPut(result, (i + 1), buff[i]);
    }

    free(buff);
    push (cIntToST(0));
    push (asOop(result));
}

// receives a byte to put into the file
void primNextPut(){
    oop fileStreamOop = getReceiver();
    int byte = (int) stIntToC(getLocal( 0));

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    int result = fputc(byte, file);
    if(result == EOF){
        LOGE("(primNextPut) could not write to file");
        push (cIntToST(1));
        push (cIntToST(-1));
    }

    push (cIntToST(0));
    push (getReceiver());
}

void primNextPutAll(){
    oop fileStreamOop = getReceiver();
    oop bufferObjOop = getLocal( 0);

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    for(int i = 0; i < basicByteSize(bufferObjOop); i++){
        fputc(basicByteAtInt(bufferObjOop, i+1), file);
    }

    push (cIntToST(0));
    push (cIntToST(0));
}

void primSkip(){
    oop fileStreamOop = getReceiver();
    int step = (int) stIntToC(getLocal( 0));

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    fseek(file, step, SEEK_CUR);

    push (cIntToST(0));
    push (fileStreamOop);
}

void primFlush(){
    oop fileStreamOop = getReceiver();

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    int result = fflush(file);

    push (cIntToST(0));
    push (cIntToST(result));
}

void primPosition(){
    oop fileStreamOop = getReceiver();

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    int result = ftell(file);

    push (cIntToST(0));
    push (cIntToST(result));
}

void primPositionColon(){
    oop fileStreamOop = getReceiver();
    int step = (int) stIntToC(getLocal( 0));

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    fseek(file, step, SEEK_SET);

    push (cIntToST(0));
    push (fileStreamOop);
}

void primUpToEnd(){
    oop fileStreamOop = getReceiver();
    size_t check;

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    long int position = ftell(file);
    fseek(file, 0, SEEK_END);
    long int endPosition = ftell(file);
    fseek(file, position, SEEK_SET);

    long int length = endPosition - position;
    oop result = newInstanceOfClass (ST_BYTE_ARRAY_CLASS, length, EdenSpace);

    char* buff = malloc(length*sizeof(char));
    check = fread(buff, sizeof(char), length,  file);

    if(check != length){
        dumpWalkback("upToEnd read past end");
        LOGE("An error has occurred while reading the file.");
		free(buff);
        push (cIntToST(0));
        push (cIntToST(check));
        return;
    }

    for(int i = 0; i < length; i++){
        basicByteAtIntPut(result, (i + 1), buff[i]);
    }

    free(buff);
    push (cIntToST(0));
    push (result);
    }

void primAtEnd() {
    oop fileStreamOop = getReceiver();

    FILE* file = (FILE *) osHandleValue (asFileStream(fileStreamOop)->handle);

    // feof only signals true after a read has failed.
    int eof = feof(file);
    if (!eof) {
        if (fgetc(file) == EOF)
            eof = TRUE;
        else
            fseek(file, -1, SEEK_CUR);
    }

    if (eof) {
        push (cIntToST(0));
        push (asOop(ST_TRUE));
    } else {
        push (cIntToST(0));
        push (asOop(ST_FALSE));
    }
}

void primExists() {
	int exists;

    oop pathOop;

	pathOop = getLocal( 0);
    char *filePath = (char *) malloc((unsigned long) basicByteSize(pathOop) + 1);

    if(STStringToC(pathOop, filePath)){
        LOGE("primExists failed");
        free(filePath);
		return;
    }

	exists = (access(filePath, F_OK) == 0);
	free (filePath);

    if (exists) {
        push (cIntToST(0));
        push (asOop(ST_TRUE));
    } else {
        push (cIntToST(0));
        push (asOop(ST_FALSE));
    }
}

void primPathDelimiter() {
        push (cIntToST(0));
#if defined(WINDOWS)
        push (CStringToST("\\"));
#else
        push (CStringToST("/"));
#endif
}


void initializeFilePrimitives() {
    primitiveTable[PRIM_OPEN] = primOpen;
    primitiveTable[PRIM_OPEN_MODE] = primOpenMode;
    primitiveTable[PRIM_CLOSE] = primClose;
    primitiveTable[PRIM_NEXT] = primNext;
    primitiveTable[PRIM_NEXT_COLON] = primNextColon;
    primitiveTable[PRIM_NEXT_PUT] = primNextPut;
    primitiveTable[PRIM_NEXT_PUT_ALL] = primNextPutAll;
    primitiveTable[PRIM_SKIP] = primSkip;
    primitiveTable[PRIM_UPTOEND] = primUpToEnd;
    primitiveTable[PRIM_AT_END] = primAtEnd;
    primitiveTable[PRIM_FLUSH] = primFlush;
    primitiveTable[PRIM_POSITION] = primPosition;
    primitiveTable[PRIM_POSITION_COLON] = primPositionColon;
    primitiveTable[PRIM_EXISTS] = primExists;
    primitiveTable[PRIM_PATH_DELIMITER] = primPathDelimiter;	
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop