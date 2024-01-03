#pragma once

// ������ ����ϱ� ���ؼ� ���̺귯�� �����ؾ� �Ѵ�
#pragma comment(lib, "ws2_32")

#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#include<process.h>
#include<vector>
#include<stack>

struct ClientInfo
{
public:
	ClientInfo()
	{
		memset(id, 0, sizeof(id));
		clientSock = NULL;
		isConnect = false;
		clientHandle = NULL;
		listenHandle = NULL;
	}

	SOCKET clientSock;
	bool isConnect;
	char id[50];
	HANDLE clientHandle;
	HANDLE listenHandle;
};

// ���ͷ����� ������ ����� ������ �� �� ����
typedef std::vector<ClientInfo>::iterator ClientIterator;

class CGameServer
{
	int portNum;
	SOCKET serverSock;
	HANDLE listenHandle;
	HANDLE mainHandle;
	bool isRun;
	SOCKET lastSock;
	std::vector<ClientInfo> clientLists;
	std::stack<ClientIterator, std::vector<ClientIterator>> clientPools;
private:
	int InitSocketLayer();
	void CloseSocketLayer();
public:
	CGameServer();
	~CGameServer();
	void Wait();
	void Listen(int nPort);
	static UINT WINAPI ListenThread(LPVOID p);
	static UINT WINAPI ControlThread(LPVOID p);
};