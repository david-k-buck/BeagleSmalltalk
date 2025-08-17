// websockets.c
//
// Beagle Smalltalk
// Copyright (c) 2025 Simberon Incorporated
// Released under the MIT License
// https://opensource.org/license/MIT

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "object.h"
#include "websockets.h"

const char base64characters[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64Encode3(char *destination, uint8_t c1, uint8_t c2, uint8_t c3)
{
	*destination++ = base64characters[c1 >> 2];
	*destination++ = base64characters[((c1 << 4) & 0x30) + (c2 >> 4)];
	*destination++ = base64characters[((c2 << 2) & 0x3C) + (c3 >> 6)];
	*destination++ = base64characters[c3 & 0x3F];
}

void base64Encode2(char *destination, uint8_t c1, uint8_t c2)
{
	*destination++ = base64characters[c1 >> 2];
	*destination++ = base64characters[((c1 << 4) & 0x30) + (c2 >> 4)];
	*destination++ = base64characters[(c2 << 2) & 0x3C];
	*destination++ = '=';
}

void base64Encode1(char *destination, uint8_t c1)
{
	*destination++ = base64characters[c1 >> 2];
	*destination++ = base64characters[(c1 << 4) & 0x30];
	*destination++ = '=';
	*destination++ = '=';
}

void base64Encode(uint8_t *source, char *destination, int length)
{
	int i;

	for (i=0; i<length; i += 3) {
		uint8_t c1, c2, c3;
		c1 = *source++;
		if (i+1 == length) {
			base64Encode1(destination, c1);
		}
		else if (i+2 == length) {
			c2 = *source++;
			base64Encode2(destination, c1, c2);
		}
		else {
			c2 = *source++;
			c3 = *source++;
			base64Encode3(destination, c1, c2, c3);
		}
		destination += 4;
	}
	if (length % 3 == 0)
		*destination++ = '=';

	*destination++ = '\0';
}

uint32_t leftRotate32(uint32_t number, int count)
{
	uint32_t temp = number << count;
	uint32_t result = number >> (32 - count) | temp;
	return (result);
}

void sha1Hash(char *source, uint32_t *hash)
{
	uint32_t length = strlen(source);
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;
	uint32_t a, b, c, d, e, f, k, w[80];
	uint64_t ml = length * 8;

	uint32_t chunkLength = (length + 1 + 8 + 63) / 64 * 64;
	uint8_t *chunk = malloc(chunkLength);
	strcpy((char *) chunk, source);
	chunk[length] = 0x80;

	int i;
	for (i = length + 1; i < (chunkLength - 8); i++)
		chunk[i] = 0;

	chunk[chunkLength - 8] = (ml >> 56) & 0xFF;
	chunk[chunkLength - 7] = (ml >> 48) & 0xFF;
	chunk[chunkLength - 6] = (ml >> 40) & 0xFF;
	chunk[chunkLength - 5] = (ml >> 32) & 0xFF;
	chunk[chunkLength - 4] = (ml >> 24) & 0xFF;
	chunk[chunkLength - 3] = (ml >> 16) & 0xFF;
	chunk[chunkLength - 2] = (ml >> 8) & 0xFF;
	chunk[chunkLength - 1] = ml & 0xFF;

	int chunkNumber;
	for (chunkNumber = 0; chunkNumber < (chunkLength / 64) ; chunkNumber++)
	{
		int wordIndex;
		for (wordIndex = 0; wordIndex < 16; wordIndex++){
			uint8_t *chunkPtr = &chunk[chunkNumber * 64 + wordIndex * 4];
			w[wordIndex] = *chunkPtr++;
			w[wordIndex] *= 256;
			w[wordIndex] += *chunkPtr++;
			w[wordIndex] *= 256;
			w[wordIndex] += *chunkPtr++;
			w[wordIndex] *= 256;
			w[wordIndex] += *chunkPtr++;
		}

		for (wordIndex = 16; wordIndex < 80; wordIndex++)
			w[wordIndex] = leftRotate32 (w[wordIndex - 3] ^ w[wordIndex - 8]
							^ w[wordIndex - 14] ^ w[wordIndex - 16], 1);

		a = h0;
		b = h1;
		c = h2;
		d = h3;
		e = h4;
		
		for (wordIndex = 0; wordIndex < 80; wordIndex++)
		{
			if (wordIndex >= 0 && wordIndex <= 19) {
				f = (b & c) | ((b ^ 0xFFFFFFFF) & d);
				k = 0x5A827999;
			}
			if (wordIndex >= 20 && wordIndex <= 39) {
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			if (wordIndex >= 40 && wordIndex <= 59) {
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			if (wordIndex >= 60 && wordIndex <= 79) {
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			uint32_t temp = leftRotate32(a, 5) + f + e + k + w[wordIndex];
			e = d;
			d = c;
			c = leftRotate32(b, 30);
			b = a;
			a = temp;
		}
	
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}
	
	hash[0] = h0;
	hash[1] = h1;
	hash[2] = h2;
	hash[3] = h3;
	hash[4] = h4;
}

#define WS_BUFFER_SIZE 16384
char wsBuffer[WS_BUFFER_SIZE];
#define WS_RESPONSE "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n"

int acceptWebSocketConnection(int connfd)
{
	char keyBuffer[128], *secKeyString="Sec-WebSocket-Key: ";
	uint32_t hash[5];
	uint8_t hashBytes[20];

	int bytes = read (connfd, wsBuffer, WS_BUFFER_SIZE), i, j;
	wsBuffer[bytes] = '\0';

	char *key = strstr(wsBuffer, secKeyString);
	key += strlen(secKeyString);
	char *keyEnd = strchr(key, '\r');
	strncpy (keyBuffer, key, (keyEnd - key));
	keyBuffer[keyEnd - key] = '\0';
	strcat(keyBuffer, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	sha1Hash(keyBuffer, hash);

	for (i=0; i<5; i++) {
		hashBytes[i*4] = hash[i] >> 24;
		hashBytes[i*4 + 1] = (hash[i] >> 16) & 0xFF;
		hashBytes[i*4 + 2] = (hash[i] >> 8) & 0xFF;
		hashBytes[i*4 + 3] = hash[i] & 0xFF;
	}
	base64Encode(hashBytes, keyBuffer, 20);

	snprintf (wsBuffer, WS_BUFFER_SIZE, WS_RESPONSE, keyBuffer);

	write (connfd, wsBuffer, strlen(wsBuffer));
	return 0;
}

int receiveWSMessage(int connfd, char *buffer, int bufferSize)
{
	int bytes = read (connfd, wsBuffer, WS_BUFFER_SIZE), i, j;
	char *ptr = wsBuffer;
	
	uint8_t mask[4];

	int opcode = *ptr++;
	int maskFlag = *ptr & 0x80;
	int length = *ptr++ & 0x7F;
	if (length == 126) {
		length = *ptr++ * 256;
		length += *ptr++;
	}
	if (length == 127) {
		length = *ptr++ * 16777216;
		length += *ptr++ * 65526;
		length += *ptr++ * 256;
		length += *ptr++;
	}

	if (maskFlag != 0) {
		mask[0] = *ptr++;
		mask[1] = *ptr++;
		mask[2] = *ptr++;
		mask[3] = *ptr++;
	}

	for (int i = 0; i < length; i++) {
		if (maskFlag != 0)
			buffer[i] = *ptr++ ^ mask[i % 4];
		else
			buffer[i] = *ptr++;
	}
	buffer[length] = '\0';

	return strlen(buffer);
}

int sendWSMessage(int connfd, char *buffer, int length)
{
	char *ptr = wsBuffer;

	*ptr++ = 0x81;
	if (length < 126) {
		*ptr++ = length;
		length += 2;
	}
	else if (length < 65536) {
		*ptr++ = 126;
		*ptr++ = length >> 8;
		*ptr++ = length & 0xFF;
		length += 4;
	}

	strcpy(ptr, buffer);
	write(connfd, wsBuffer, length);
	return 0;
}

int closeWebSocket(int connfd)
{
	return 0;
}
