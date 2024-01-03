// ������ ����ϱ� ���ؼ� ���̺귯�� �����ؾ� �Ѵ�
#pragma comment(lib, "ws2_32")
// inet_ntoa�� deprecated�� �Ǿ��µ� ����Ϸ��� �Ʒ� ������ �ؾ��Ѵ�
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

	// ������ ���� ���� ���� ���̺귯�� �ʱ�ȭ
	// ������ ���� ������ ���ø����̼� �䱸������ �����ϴ��� Ȯ���Ѵ�
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		printf("WSAStartup Error\n");
		return -1;
	}

	// ���� ���� : socket(int domain, int type, int protocol)
	// PF_INET : IPv4 ���ͳ� ��������, PF_INET6 : IPv6 ���ͳ� ��������
	// SOCKET_STREAM : TCP/IP ��������, SOCK_DGRAM UDP/IP ��������
	// ��ſ� �־� Ư�� �������� ����ϱ� ���� ������ ���� 0 ���
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("socket ���� ���� \n");
		return -1;
	}

	memset(&ServAddr, 0, sizeof(ServAddr));
	// PF_INET�� �������� ü��, AF_INET�� �ּ� ü��
	// AF_INET�� IPv4, AF_INET6�� IPv6, AF_LOCAL�� ��ſ�
	ServAddr.sin_family = AF_INET;
	// INADDR_ANY�� ������ IP�ּҸ� �ڵ����� ã�Ƽ� �������ִ� �Լ�
	// IP�ּҸ� INADDR_ANY��, ��Ʈ ��ȣ�� 9000���� �� ��� => ���� ���� ��ǻ���� 9000�� ��Ʈ�� �������� �ϴ� ��� ���� ��û�� �ش� ���� �������α׷����� ó���ϰڴٴ� �ǹ�
	ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// ����� ��Ʈ ��ȣ ����
	ServAddr.sin_port = htons(portnum);

	// ������ ������ �̿��Ͽ� ������ ���� ��û
	if (connect(sock, (SOCKADDR*)&ServAddr, sizeof(ServAddr)) == SOCKET_ERROR)
	{
		printf("connect Error\n");
		return -1;
	}

	// recv ������ ����
	hThread = (HANDLE)_beginthreadex(NULL, 0, Run, (void*)sock, CREATE_SUSPENDED, NULL);
	if (!hThread)
	{
		printf("������ ���� \r\n");
		return -1;
	}
	g_IsRunning = true;
	ResumeThread(hThread);

	while (g_IsRunning == true)
	{
		printf("�޽����� �Է��ϼ��� : ");
		gets_s(recvMessage, sizeof(recvMessage));
		printf("����ڷκ��� ���� �޽��� : %s\n", recvMessage);
		printf("������ �޽����� �����մϴ�\n");

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
	// recv �κи� �̵�
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
				printf("Ŭ���̾�Ʈ���� ���� �޽��� : %s\n", extraPacket);
				break;
			}
		}
	}

	return 0;
}