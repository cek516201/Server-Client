// 소켓을 사용하기 위해서 라이브러리 참조해야 한다
#pragma comment(lib, "ws2_32")
// inet_ntoa가 deprecated가 되었는데 사용하려면 아래 설정을 해야한다
#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <process.h>
#include "CVSP.h"

unsigned __stdcall Run(void* p);
bool g_IsRunning;


int main(int argc, char** argv)
{
	WSADATA wsadata;
	SOCKET sock;
	SOCKADDR_IN ServAddr;
	int portnum = 5004;
	HANDLE hThread;

	unsigned char cmd;
	unsigned char option;
	int len;
	char recvMessage[100];

	// 윈도우 소켓 동적 연결 라이브러리 초기화
	// 윈도우 소켓 구현이 애플리케이션 요구사항을 충족하는지 확인한다
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		printf("WSAStartup Error\n");
		return -1;
	}

	// 소켓 생성 : socket(int domain, int type, int protocol)
	// PF_INET : IPv4 인터넷 프로토콜, PF_INET6 : IPv6 인터넷 프로토콜
	// SOCKET_STREAM : TCP/IP 프로토콜, SOCK_DGRAM UDP/IP 프로토콜
	// 통신에 있어 특정 프로토콜 사용하기 위한 변수로 보통 0 사용
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("socket 생성 실패 \n");
		return -1;
	}

	memset(&ServAddr, 0, sizeof(ServAddr));
	// PF_INET은 프로토콜 체계, AF_INET은 주소 체계
	// AF_INET은 IPv4, AF_INET6은 IPv6, AF_LOCAL은 통신용
	ServAddr.sin_family = AF_INET;
	// INADDR_ANY는 서버의 IP주소를 자동으로 찾아서 대입해주는 함수
	// IP주소를 INADDR_ANY로, 포트 번호를 9000으로 할 경우 => 현재 서버 컴퓨터의 9000번 포트를 목적지로 하는 모든 연결 요청을 해당 서버 응용프로그램에서 처리하겠다는 의미
	ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// 사용할 포트 번호 지정
	ServAddr.sin_port = htons(portnum);

	// 생성된 소켓을 이용하여 서버에 연결 요청
	if (connect(sock, (SOCKADDR*)&ServAddr, sizeof(ServAddr)) == SOCKET_ERROR)
	{
		printf("connect Error\n");
		return -1;
	}

	// recv 스레드 생성
	hThread = (HANDLE)_beginthreadex(NULL, 0, Run, (void*)sock, CREATE_SUSPENDED, NULL);
	if (!hThread)
	{
		printf("쓰레드 에러 \r\n");
		return -1;
	}
	g_IsRunning = true;
	ResumeThread(hThread);

	while (g_IsRunning == true)
	{
		printf("메시지를 입력하세요 : ");
		gets_s(recvMessage, sizeof(recvMessage));
		printf("사용자로부터 받은 메시지 : %s\n", recvMessage);
		printf("서버로 메시지를 전송합니다\n");

		if (strcmp(recvMessage, "exit") == 0)
		{
			cmd = CVSP_LEAVEREQ;
			option = CVSP_SUCCESS;
		}
		else
		{
			cmd = CVSP_CHATTINGREQ;
			option = CVSP_SUCCESS;
		}

		if (sendCVSP((unsigned int)sock, cmd, option, recvMessage, strlen(recvMessage)) < 0)
		{
			printf("Send Error!\n");
		}

		if (strcmp(recvMessage, "exit") == 0)
		{
			g_IsRunning = false;
			break;
		}
	}

	printf("SOCKET OVER\n");
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	closesocket(sock);
	WSACleanup();

	return 0;
}

unsigned __stdcall Run(void* p)
{
	// recv 부분만 이동
	SOCKET CliSock = (SOCKET)p;
	unsigned char cmd;
	unsigned char option;
	int len;
	char extraPacket[CVSP_STANDARD_PAYLOAD_LENGTH - sizeof(CVSPHeader_t)];

	while (g_IsRunning == true)
	{
		memset(extraPacket, 0, sizeof(extraPacket));
		len = recvCVSP(CliSock, &cmd, &option, extraPacket, sizeof(extraPacket));
		if (len == SOCKET_ERROR)
		{
			printf("recv Error\n");
			return -1;
		}

		switch (cmd)
		{
			case CVSP_CHATTINGRES:
			{
				printf("클라이언트에서 받은 메시지 : %s\n", extraPacket);
				break;
			}
		}
	}

	return 0;
}