#include "stdafx.hpp"

void HandleGetDir(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest req) {

}

void HandleBye(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest req) {
	int n;
	FSPRequest sReq = FSPRequest::Create(CC_BYE, NULL, 0, 0, req.pHdr->sequence);
	PBYTE sBuf = (PBYTE)malloc(FSP_MAXSPACE);
	UINT32 sSize = sReq.Pack(sBuf);
	realloc(sBuf, sSize);
	if((n = sendto(srvSock, sBuf, sSize, 0, cliAddr, cliAddrLen)) == -1) {

	}
}

int main(int argc, char* argv[]) {
	printf("FSPD-Plus-Plus server started on %s:%i...\n", SERVER_ADDR, SERVER_PORT);

	int srvSock;
	int opt = TRUE;
	sockaddr_in srvAddr;
	memset(&srvAddr, 0, sizeof(srvAddr));
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

	// receive buffer size
	opt = FSP_MAXSPACE;
	if(setsockopt(srvSock, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt))) {
		perror("setsockopt rcvbuf failed!\n");
		return ERROR_SETSOCKOPT_FAILED;
	}

	// send buffer size
	if(setsockopt(srvSock, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt))) {
		perror("setsockopt sndbuf failed!\n");
		return ERROR_SETSOCKOPT_FAILED;
	}

	if(bind(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr))) {
		perror("socket bind failed!\n");
		return ERROR_SOCKET_BIND_FAILED;
	}

	// receive in a loop on a single thread...
	// terrible for multiple clients but oh well :/
	printf("Waiting on client...\n");
	while(true) {
		// receive variables
		int len, n;
		BYTE recvBuf[FSP_MAXSPACE];
		memset(recvBuf, 0, FSP_MAXSPACE);

		sockaddr_in cliAddr;
		memset(&cliAddr, 0, sizeof(cliAddr));
		if((n = recvfrom(srvSock, recvBuf, FSP_MAXSPACE, 0, (sockaddr*)&cliAddr, (socklen_t*)&len)) == -1) {
			perror("recvfrom failed!\n");
			return ERROR_RECEIVE_FAILED;
		}

		// check bounds
		if(!(n > 0 && n <= FSP_MAXSPACE)) {
			perror("receive size out of bounds!\n");
			return ERROR_SIZE_OUT_OF_BOUNDS;
		}

		// print client info
		char addrStr[IP_ADDR_MAX_LEN] = { 0 };
		Utils::LongToAddress(addrStr, cliAddr.sin_addr.s_addr);
		printf("%s:%i\n", addrStr, ntohs(cliAddr.sin_port));
		
		// parse the packet
		FSPRequest req = FSPRequest::Parse(recvBuf, n);

		printf("Command: %02X\n", req.pHdr->command);
		Utils::PrintHex(recvBuf, n);

		// check password
		switch(req.pHdr->command) {
			case CC_GET_DIR:
			case CC_GET_FILE:
			case CC_STAT:
			case CC_DEL_FILE:
			case CC_INSTALL: {
				// check the password
				if(strcmp(req.pcPassword, Globals::FSP_PASSWORD) != 0) {
					printf("Invalid password attempted: \"%s\"\n", req.pcPassword);
					continue;
				}
				break;
			}
			default: {
				break;
			}
		}

		// handle commands
		switch(req.pHdr->command) {
			case CC_GET_DIR: {
				HandleGetDir(srvSock, (sockaddr*)&cliAddr, len, req);
				break;
			}
			case CC_BYE: {
				HandleBye(srvSock, (sockaddr*)&cliAddr, len, req);
				break;
			}
			default: {
				break;
			}
		}
	}

	return ERROR_NONE;
}
