#include "stdafx.hpp"

static void* ReceiveClient(void* vpargs) {
	PRECEIVE_CLIENT_ARGS pargs = (PRECEIVE_CLIENT_ARGS)vpargs;

	printf("%i\n", pargs->sock);

	// receive variables
	int len, n;
	BYTE buf[FSP_MAXSPACE];
	memset(buf, 0, FSP_MAXSPACE);

	sockaddr_in cliAddr;
	memset((PBYTE)&cliAddr, 0, sizeof(cliAddr));
	if((n = recvfrom(pargs->sock, buf, FSP_MAXSPACE, 0, (sockaddr*)&cliAddr, (socklen_t*)&len)) == -1) {
		PFSP_HDR phdr = (PFSP_HDR)buf;
		PBYTE pbFspData = buf + sizeof(FSP_HDR);
		PBYTE pbFspExtra = buf + sizeof(FSP_HDR) + phdr->length;
		// create a copy since this will be nulled when it's checked
		BYTE storCksm = phdr->checksum;
		BYTE calcCksm = Utils::CalcClientToServerChecksum(buf, sizeof(FSP_HDR) + phdr->length);
		if(storCksm != calcCksm)
			printf("Checksum mismatch!\n");
	}

	return NULL;
}

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

	printf("%i\n", srvSock);

	pthread_t t;
	RECEIVE_CLIENT_ARGS args;
	args.sock = srvSock;
	int err = pthread_create(&t, NULL, &ReceiveClient, &args);
	if(err) {
		perror("thread creation failed!\n");
		return ERROR_THREAD_CREATE_FAILED;
	}

	err = pthread_detach(t);
	if(err) {
		perror("thread detech failed!\n");
		return ERROR_THREAD_DETECH_FAILED;
	}

	/* err = pthread_join(t, NULL);
	if(err) {
		perror("thread join failed!\n");
		return ERROR_THREAD_JOIN_FAILED;
	} */

	return ERROR_NONE;
}
