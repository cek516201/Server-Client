#ifndef _CVSP_1_0_0_H_
#define _CVSP_1_0_0_H_

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define WIN32 1

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#include <winsock.h>
#include <io.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <unistd.h>
#endif
#endif

#define CVSP_MONITORING_MESSAGE 700
#define CVSP_MONITORING_LOAD 701

// Version
#define CVSP_VER 0x01

// Port
#define CVSP_PORT 9000

// Payload Size
#define CVSP_STANDARD_PAYLOAD_LENGTH 4096

// Command
#define CVSP_JOINREQ 0x01
#define CVSP_JOINRES 0x02
#define CVSP_CHATTINGREQ 0x03
#define CVSP_CHATTINGRES 0x04
#define CVSP_OPERATINGREQ 0x05
#define CVSP_MONITORINGMSG 0x06
#define CVSP_LEAVEREQ 0x07

// Option
#define CVSP_SUCCESS 0x01
#define CVSP_FAIL 0x02

// Variable Style
#ifdef WIN32
typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned long u_long;
#endif

// Header(32bit)
typedef struct CVSPHeader_t
{
	u_char cmd;
	u_char option;
	u_short packetLength;
} CVSPHeader;

// Version
#define CVSP_VER 0x01

// API Interface
int sendCVSP(unsigned int sockfd, unsigned char cmd, unsigned char option, void* payload, unsigned short len);
int recvCVSP(unsigned int sockfd, unsigned char* cmd, unsigned char* option, void* payload, unsigned short len);

// 여기에 통신할 데이터들을 정의할 수 있음