#include"GameServer.h"

int main(int argc, char** argv)
{
	CGameServer server;
	server.Listen(5004);
	server.Wait();

	return 0;
}