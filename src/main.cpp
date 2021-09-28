#include "stdafx.hpp"

int main(int argc, char* argv[]) {
	printf("FSPD-Plus-Plus server started on %s:%i...\n", SERVER_ADDR, SERVER_PORT);

	int srvSock;
	int opt = TRUE;
	sockaddr_in srvAddr;
	memset((PBYTE)&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(SERVER_PORT);
	srvAddr.sin_addr.s_addr = Utils::AddressToLong((const PCHAR)SERVER_ADDR);

	if((srvSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket creation failed!\n");
		return ERROR_SOCKET_CREATE_FAILED;
	}

	if(setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt reuseaddr and reuseport failed!\n");
		return ERROR_SETSOCKOPT_FAILED;
	}

	opt = FSP_MAXSPACE;
	if(setsockopt(srvSock, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt))) {
		perror("setsockopt rcvbuf failed!\n");
		return ERROR_SETSOCKOPT_FAILED;
	}

	if(setsockopt(srvSock, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt))) {
		perror("setsockopt sndbuf failed!\n");
		return ERROR_SETSOCKOPT_FAILED;
	}

	if(bind(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr))) {
		perror("socket bind failed!\n");
		return ERROR_SOCKET_BIND_FAILED;
	}

	// receive variables
	int len, n;
	BYTE buf[FSP_MAXSPACE];
	memset(buf, 0, FSP_MAXSPACE);

	sockaddr_in cliAddr;
	memset((PBYTE)&cliAddr, 0, sizeof(cliAddr));
	n = recvfrom(srvSock, buf, FSP_MAXSPACE, 0, (sockaddr*)&cliAddr, (socklen_t*)&len);

	return ERROR_NONE;
}
