// websockets.h
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

extern int acceptWebSocketConnection(int connfd);
extern int receiveWSMessage(int connfd, char *buffer, int bufferSize);
extern int sendWSMessage(int connfd, char *buffer, int bufferSize);
extern int closeWebSocket(int connfd);

