#include "stdafx.hpp"

/* References:
https://sourceforge.net/p/fsp/code/ci/master/tree/doc/PROTOCOL
https://github.com/emukidid/swiss-gc/blob/master/cube/swiss/source/devices/fsp/deviceHandler-FSP.c
https://github.com/emukidid/swiss-gc/blob/master/cube/swiss/source/devices/fsp/fsplib.c
*/

VOID HandleGetDir(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	// create full path to the FSP directory
	fs::path p = Utils::RebasePath(pReq->pcDirectory);
	printf("HandleGetDir: %s\n", p.c_str());

	// packet vars
	UINT32 pktNum = pReq->pHdr->position / FSP_SPACE;
	UINT32 pktOff = pReq->pHdr->position % FSP_SPACE;

	if(strcmp(Cache::FspLastGetDir, "") == 0 || Cache::PktQueue.size() == 0 || strcmp(p.c_str(), Cache::FspLastGetDir) != 0) {
		Cache::FspLastGetDir = (PCHAR)p.c_str();

		// clear packet queue
		Utils::ClearVector(&Cache::PktQueue);

		// entry queue
		vector<vector<BYTE>> ents;

		for (const auto & entry : fs::directory_iterator(p)) {
			vector<BYTE> ent;
			if(fs::is_directory(entry.path()) || fs::is_regular_file(entry.path())) {
				RDIRENT* pEnt = RDIRENT::Create((PCHAR)entry.path().c_str());
				Alloc* pAlloc = pEnt->Pack();
				Utils::CopyToVector(&ent, (PBYTE)pAlloc->GetAddr(), pAlloc->GetSize());
				delete pAlloc;
				delete pEnt;
				ents.push_back(ent);
			}
		}

		// create end
		vector<BYTE> ent;
		RDIRENT* pEnt = RDIRENT::CreateEnd();
		Alloc* pAlloc = pEnt->Pack();
		Utils::CopyToVector(&ent, (PBYTE)pAlloc->GetAddr(), pAlloc->GetSize());
		ents.push_back(ent);
		delete pAlloc;

		// loop through each entry and create a packet
		while(TRUE) {
			// create new packet buffer
			vector<BYTE> pkt;
			while(ents.size() > 0) {
				// grab first RDIRENT
				ent = ents.front();
				// remove it from the vector
				ents.erase(ents.begin());
				// entry will overlap directory block boundary
				if(pkt.size() + ent.size() > FSP_SPACE) {
					if(pkt.size() + sizeof(RDIRENT_HDR) > FSP_SPACE) {
						// nothing
					} else {
						RDIRENT* pEnt = RDIRENT::CreateSkip();
						Alloc* pAlloc = pEnt->Pack();
						Utils::CopyToVector(&ent, (PBYTE)pAlloc->GetAddr(), pAlloc->GetSize());
						delete pAlloc;
						delete pEnt;
					}
					// pad to boundary
					for(int i = 0; i < Utils::CalcPadSize(pkt.size(), FSP_SPACE); i++) {
						pkt.push_back(0);
					}
					// add the packed back to the beginning of the queue
					ents.insert(ents.begin(), ent);
					// packet is full
					break;
				} else if(pkt.size() + ent.size() <= FSP_SPACE) {
					Utils::CopyToVector(&pkt, ent.data(), ent.size());
				}
			}

			// add packet to send queue
			Cache::PktQueue.push_back(pkt);

			// leave the loop since the queue is empty
			if(ents.size() == 0)
				break;
		}
	}

	if(strcmp(p.c_str(), Cache::FspLastGetDir) == 0 && Cache::PktQueue.size() > 0) {
		int n;
		FSPRequest* pSndReq = FSPRequest::Create(pReq->pHdr->command, (Cache::PktQueue[pktNum].data() + pktOff), (Cache::PktQueue[pktNum].size() - pktOff), NULL, 0, pReq->pHdr->position, pReq->pHdr->sequence);
		pSndReq->PackAndSend(srvSock, cliAddr, cliAddrLen);
		
		delete pSndReq;

		// clear cache since we've sent all the packets
		if((pktNum + 1) == Cache::PktQueue.size() && Cache::PktQueue[pktNum].size() - pktOff <= FSP_SPACE)
			Utils::ClearVector(&Cache::PktQueue);
	}
}

VOID HandleGetFile(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	// create full path to the FSP directory
	fs::path p = Utils::RebasePath(pReq->pcFilename);
	printf("HandleGetFile: %s\n", p.c_str());

	// set block size
	pReq->blkSize = FSP_SPACE;

	printf("Command:  0x%02X\n", pReq->pHdr->command);
	printf("Checksum: 0x%02X\n", pReq->pHdr->checksum);
	printf("Key:      0x%04X\n", pReq->pHdr->key);
	printf("Sequence: 0x%04X\n", pReq->pHdr->sequence);
	printf("Length:   0x%04X\n", pReq->pHdr->length);
	printf("Position: 0x%08X\n", pReq->pHdr->position);

	PBYTE pbData = (PBYTE)calloc(1, pReq->blkSize);
	FILE* f = fopen(p.c_str(), "rb");
	fseek(f, pReq->pHdr->position, SEEK_SET);
	int bRead = fread(pbData, sizeof(BYTE), pReq->blkSize, f);
	fclose(f);
	pbData = (PBYTE)realloc(pbData, bRead);

	FSPRequest* pSndReq = FSPRequest::Create(pReq->pHdr->command, pbData, bRead, NULL, 0, pReq->pHdr->position, pReq->pHdr->sequence);
	pSndReq->PackAndSend(srvSock, cliAddr, cliAddrLen);

	// cleanup
	free(pbData);
	delete pSndReq;
}

VOID HandleStat(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	// create full path to the FSP directory
	fs::path p = Utils::RebasePath(pReq->pcFilename);

	STAT* pStat = STAT::Create((PCHAR)p.c_str());
	Alloc* pAllocS = pStat->Pack();

	FSPRequest* pSndReq = FSPRequest::Create(pReq->pHdr->command, (PBYTE)pAllocS->GetAddr(), pAllocS->GetSize(), NULL, 0, pReq->pHdr->position, pReq->pHdr->sequence);
	pSndReq->PackAndSend(srvSock, cliAddr, cliAddrLen);

	// cleanup
	delete pAllocS;
	delete pSndReq;
}

VOID HandleBye(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	FSPRequest* pSndReq = FSPRequest::Create(CC_BYE, NULL, 0, NULL, 0, pReq->pHdr->position, pReq->pHdr->sequence);
	pSndReq->PackAndSend(srvSock, cliAddr, cliAddrLen);

	// cleanup
	delete pSndReq;
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
		PBYTE pbRcvData = (PBYTE)calloc(1, FSP_MAXSPACE);
		memset(pbRcvData, 0, FSP_MAXSPACE);

		sockaddr_in cliAddr;
		memset(&cliAddr, 0, sizeof(cliAddr));
		if((n = recvfrom(srvSock, pbRcvData, FSP_MAXSPACE, 0, (sockaddr*)&cliAddr, (socklen_t*)&len)) == -1) {
			perror("recvfrom failed!\n");
			return ERROR_RECEIVE_FAILED;
		}
		pbRcvData = (PBYTE)realloc(pbRcvData, n);

		// check bounds
		if(!(n > 0 && n <= FSP_MAXSPACE)) {
			perror("receive size out of bounds!\n");
			free(pbRcvData);
			return ERROR_SIZE_OUT_OF_BOUNDS;
		}

		// print client info
		//char addrStr[IP_ADDR_MAX_LEN] = { 0 };
		//Utils::LongToAddress(addrStr, cliAddr.sin_addr.s_addr);
		//printf("%s:%i\n", addrStr, ntohs(cliAddr.sin_port));
		
		// parse the packet
		FSPRequest* pReq = FSPRequest::Parse(pbRcvData, n);
		// free packet data
		free(pbRcvData);

		// check password
		switch(pReq->pHdr->command) {
			case CC_GET_DIR:
			case CC_GET_FILE:
			case CC_STAT:
			case CC_DEL_FILE:
			case CC_INSTALL: {
				// check the password
				if(strcmp(pReq->pcPassword, Globals::FSP_PASSWORD) != 0) {
					printf("Invalid password attempted: \"%s\"\n", pReq->pcPassword);
					continue;
				}
				break;
			}
			default: {
				break;
			}
		}

		// handle commands
		switch(pReq->pHdr->command) {
			case CC_GET_DIR: {
				HandleGetDir(srvSock, (sockaddr*)&cliAddr, len, pReq);
				break;
			}
			case CC_GET_FILE: {
				HandleGetFile(srvSock, (sockaddr*)&cliAddr, len, pReq);
				break;
			}
			case CC_STAT: {
				HandleStat(srvSock, (sockaddr*)&cliAddr, len, pReq);
				break;
			}
			case CC_BYE: {
				HandleBye(srvSock, (sockaddr*)&cliAddr, len, pReq);
				break;
			}
			default: {
				printf("Unhandled Request:\n");
				printf("Command:  0x%02X\n", pReq->pHdr->command);
				printf("Checksum: 0x%02X\n", pReq->pHdr->checksum);
				printf("Key:      0x%04X\n", pReq->pHdr->key);
				printf("Sequence: 0x%04X\n", pReq->pHdr->sequence);
				printf("Length:   0x%04X\n", pReq->pHdr->length);
				printf("Position: 0x%08X\n", pReq->pHdr->position);
				break;
			}
		}

		delete pReq;
	}

	return ERROR_NONE;
}
