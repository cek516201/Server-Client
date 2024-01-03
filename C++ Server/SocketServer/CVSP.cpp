#include "CVSP.h"

int sendCVSP(unsigned int sockfd, unsigned char cmd, unsigned char option, void* payload, unsigned short len)
{
	char *CVSPPacket;
	CVSPHeader_t CVSPHeader;
	u_int packetSize;
	int result;

	// 패킷 사이즈 설정
	packetSize = len + sizeof(CVSPHeader_t);

	// 헤더파일을 만듬
	CVSPHeader.cmd = cmd;
	CVSPHeader.option = option;
	CVSPHeader.packetLength = packetSize;

	CVSPPacket = (char*)malloc(packetSize);
	assert(CVSPPacket); // 패킷에 문제가 있는지 확인

	memset(CVSPPacket, 0, packetSize); // 패킷 초기화
	memcpy(CVSPPacket, &CVSPHeader, sizeof(CVSPHeader_t));

	if (payload != NULL) // 페이로드가 비어있지 않아야 복사를 수행
	{
		memcpy(CVSPPacket + sizeof(CVSPHeader_t), payload, len);
	}

	result = send(sockfd, CVSPPacket, packetSize, 0);
	if (result < 0)
	{
		return result;
	}

	free(CVSPPacket);

	return result;
}

int recvCVSP(unsigned int sockfd, unsigned char* cmd, unsigned char* option, void* payload, unsigned short len)
{
	CVSPHeader_t CVSPHeader;
	char extraPacket[CVSP_STANDARD_PAYLOAD_LENGTH];
	int recvSize;
	int payloadSize;
	int payloadCopySize;

	assert(payload != NULL); // 패킷 검증부터 하기

	memset(extraPacket, 0, sizeof(extraPacket));

	int last_readn, cur_readn;
	int ret = 0;

	recvSize = recv(sockfd, (char*)&CVSPHeader, sizeof(CVSPHeader), MSG_PEEK);
	if (recvSize < 0)
	{
		return recvSize;
	}

	last_readn = CVSPHeader.packetLength;
	cur_readn = 0;

	// 헤더에 명시된 사이즈만큼 패킷을 읽어냄
	// 나눠서 읽을 수 있음
	for (; cur_readn != CVSPHeader.packetLength;)
	{
		ret = recv(sockfd, extraPacket + cur_readn, last_readn, 0);
		if (ret < 0)
		{
			return -1;
		}

		last_readn -= ret;
		cur_readn += ret;
	}

	// header copy
	memcpy(&CVSPHeader, extraPacket, sizeof(CVSPHeader));
	payloadSize = CVSPHeader.packetLength - sizeof(CVSPHeader_t);
	*cmd = CVSPHeader.cmd;
	*option = CVSPHeader.option;
	payloadCopySize = payloadSize;

	// payload copy
	if (payloadCopySize != 0)
	{
		memcpy(payload, extraPacket + sizeof(CVSPHeader), payloadCopySize);
	}

	return payloadCopySize;
}