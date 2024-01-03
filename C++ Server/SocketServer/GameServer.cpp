#include "GameServer.h"
#include <sys\types.h>
#include <sys\stat.h>
#include <Mmsystem.h>
#include "CVSP.h"

CGameServer::CGameServer()
{
	portNum = 0;
	isRun = true;
	InitSocketLayer();

	for (int i = 0; i < 100; i++)
	{
		ClientInfo info;
		sprintf_s(info.id, "%d", i);
		clientLists.push_back(info);
	}

	ClientIterator itr;
	itr = clientLists.begin();
	while (itr != clientLists.end())
	{
		clientPools.push(itr);
		++itr;
	}

}

CGameServer::~CGameServer()
{
	isRun = false;
	while (!clientPools.empty()) clientPools.pop();

	ClientIterator itr;
	itr = clientLists.begin();
	while (itr != clientLists.end())
	{
		if (itr->isConnect == true)
		{
			itr->isConnect = false;
			WaitForSingleObject(itr->clientHandle, INFINITE);
			CloseHandle(itr->clientHandle);
			closesocket(itr->clientSock);
		}
		++itr;
	}
	clientLists.clear();
	WaitForSingleObject(listenHandle, INFINITE);
	CloseHandle(listenHandle);
	closesocket(serverSock);
	CloseSocketLayer();

}


int CGameServer::InitSocketLayer()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		printf("WSAStartup 에러!\n");
		return -1;
	}

	return 0;
}

void CGameServer::CloseSocketLayer()
{
	WSACleanup();
}

void CGameServer::Wait()
{
	WaitForSingleObject(listenHandle, INFINITE);

}

void CGameServer::Listen(int nPort)
{
	portNum = nPort;
	listenHandle = (HANDLE)_beginthreadex(NULL, 0, CGameServer::ListenThread, this, 0, NULL);

	if (!listenHandle)
	{
		printf("쓰레드에러! \n");
		return;
	}
}


UINT WINAPI CGameServer::ListenThread(LPVOID p)
{
	CGameServer* pServer;
	pServer = (CGameServer*)p;

	pServer->serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pServer->serverSock == INVALID_SOCKET)
	{
		WSACleanup();
		return -1;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(pServer->portNum);

	if (bind(pServer->serverSock, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		closesocket(pServer->serverSock);
		return -1;
	}

	if (listen(pServer->serverSock, 5) == SOCKET_ERROR)
	{
		closesocket(pServer->serverSock);
		return -1;
	}


	while (pServer->isRun)
	{
		SOCKET connectSock;
		connectSock = accept(pServer->serverSock, NULL, NULL);

		if (connectSock > 0)
		{
			if (pServer->clientPools.empty())
			{
				closesocket(connectSock);
				continue;
			}
			else
			{
				ClientIterator itr;
				itr = pServer->clientPools.top();
				pServer->clientPools.pop();
				itr->clientSock = connectSock;
				printf("클라이언트연결 소켓%d 내부관리번호%s\n", itr->clientSock, itr->id);
				pServer->lastSock = connectSock;
				itr->isConnect = true;
				itr->clientHandle = (HANDLE)_beginthreadex(NULL, 0, CGameServer::ControlThread, pServer, 0, NULL);
			}
		}

		Sleep(50);
	}

	return 0;
}


UINT WINAPI CGameServer::ControlThread(LPVOID p)
{
	CGameServer* pServer;
	pServer = (CGameServer*)p;
	SOCKET connectSock = pServer->lastSock;

	bool bFound = false;

	ClientIterator itr;
	itr = pServer->clientLists.begin();
	while (itr != pServer->clientLists.end())
	{
		if (itr->clientSock == connectSock)
		{
			bFound = true;
			//char message[] = "Hoseo Game!";
			//send(connectSock, message, sizeof(message), 0);
			printf("매칭 성공! 클라이언트연결 소켓%d 내부관리번호%s\n", itr->clientSock, itr->id);
			break;
		}

		++itr;
	}

	// select 설정들
	fd_set fdReadSet, fdErrorSet, fdMaster;
	struct timeval tvs;
	unsigned char cmd;
	unsigned char option;
	int len;
	char extraPacket[CVSP_STANDARD_PAYLOAD_LENGTH - sizeof(CVSPHeader_t)];

	FD_ZERO(&fdMaster);
	FD_SET(connectSock, &fdMaster);
	tvs.tv_sec = 0;
	tvs.tv_usec = 100;

	while (itr->isConnect && bFound)
	{
		fdReadSet = fdMaster;
		fdErrorSet = fdMaster;
		select((int)connectSock + 1, &fdReadSet, NULL, &fdErrorSet, &tvs);

		if (FD_ISSET(connectSock, &fdReadSet))
		{
			memset(extraPacket, 0, sizeof(extraPacket));
			//len = recv(connectSock, recvMessage, sizeof(recvMessage) - 1, 0);
			len = recvCVSP((unsigned int)connectSock, &cmd, &option, extraPacket, sizeof(extraPacket));
			if (len == SOCKET_ERROR)
			{
				printf("recv Error!\n");
				break;
			}

			switch (cmd)
			{
				case CVSP_JOINREQ:
				{
					// 사실 여기서 DBMS 연동을 해서 아이디/패스워드를 확인해야하지만 여기선 간략히 로그인처리만 하자
					printf("클라이언트에서 아이디:%s\n", extraPacket);
					sprintf_s(itr->id, sizeof(itr->id), "%s", extraPacket);

					cmd = CVSP_JOINRES;
					option = CVSP_SUCCESS;
					if (sendCVSP((unsigned int)itr->clientSock, cmd, option, NULL, 0) < 0)
					{
						printf("Send CVSP Error!\n");
					}
				}

				case CVSP_CHATTINGREQ:
				{
					printf("클라이언트에서 받은 메시지:%s\n", extraPacket);

					// 여기서 접속한 모든 클라이언트들에게 메시지를 뿌림
					ClientIterator citr;
					citr = pServer->clientLists.begin();
					while (citr != pServer->clientLists.end())
					{
						if (citr->isConnect)
						{
							printf("메시지 재전송..클라이언트연결 소켓%d 내부관리번호%s\n", citr->clientSock, citr->id);
							cmd = CVSP_CHATTINGRES;
							option = CVSP_SUCCESS;

							if (sendCVSP((unsigned int)citr->clientSock, cmd, option, extraPacket, strlen(extraPacket)) < 0)
							{
								printf("Send Error!\n");
							}
						}

						++citr;
					}

					break;
				}

				case CVSP_LEAVEREQ:
				{
					printf("소켓 연결을 종료합니다...\n");
					itr->isConnect = false;

					break;
				}
			}
		}
	}

	closesocket(itr->clientSock);
	pServer->clientPools.push(itr);
	printf("클라이언트 연결이 종료 되었습니다!\n");

	return 0;
}