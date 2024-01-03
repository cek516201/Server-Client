#pragma once

// 소켓을 사용하기 위해서 라이브러리 참조해야 한다
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

// 이터레이터 선언을 해줘야 관리를 할 수 있음
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