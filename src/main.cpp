#include "stdafx.hpp"

// packet queue
vector<vector<BYTE>> pkts;
PCHAR LastGetDir = "";

void HandleGetDir(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	// packet vars
	UINT32 pktNum = pReq->pHdr->position / FSP_SPACE;
	UINT32 pktOff = pReq->pHdr->position % FSP_SPACE;

	// create full path to the FSP directory
	fs::path p(Globals::FSP_DIRECTORY);
	p = fs::absolute(p);
	if(strcmp(pReq->pcDirectory, "/") != 0)
		p /= pReq->pcDirectory;
	
	if(LastGetDir == "" || pkts.size() == 0 || pReq->pcDirectory != LastGetDir) {
		LastGetDir = pReq->pcDirectory;

		// clear packet queue
		pkts.empty();

		// entry queue
		vector<vector<BYTE>> ents;

		for (const auto & entry : fs::directory_iterator(p)) {
			vector<BYTE> ent;
			if(fs::is_directory(entry.path())) {
				RDIRENT* pEnt = RDIRENT::Create((PCHAR)entry.path().c_str());
				UINT32 cbData;
				PBYTE pbData = pEnt->Pack(&cbData);
				Utils::CopyToVector(&ent, pbData, cbData);
				free(pbData);
				delete pEnt;
			} else if(fs::is_regular_file(entry.path())) {
				RDIRENT* pEnt = RDIRENT::Create((PCHAR)entry.path().c_str());
				UINT32 cbData;
				PBYTE pbData = pEnt->Pack(&cbData);
				Utils::CopyToVector(&ent, pbData, cbData);
				free(pbData);
				delete pEnt;
			}
			ents.push_back(ent);
		}

		// create end
		vector<BYTE> ent;
		RDIRENT* pEnt = RDIRENT::CreateEnd();
		UINT32 cbData;
		PBYTE pbData = pEnt->Pack(&cbData);
		Utils::CopyToVector(&ent, pbData, cbData);
		ents.push_back(ent);
		free(pbData);

		// loop through each entry and create a packet
		while(true) {
			vector<BYTE> pkt;

			while(ents.size() > 0) {
				// grab first RDIRENT
				ent = ents.front();
				// remove it from the vector
				ents.erase(ents.begin());

				if(pkt.size() + ent.size() > FSP_SPACE) {
					if(pkt.size() + sizeof(RDIRENT_HDR) > FSP_SPACE) {
						
					} else {
						RDIRENT* pEnt = RDIRENT::CreateSkip();
						UINT32 cbSize;
						PBYTE pbData = pEnt->Pack(&cbSize);
						Utils::CopyToVector(&pkt, pbData, cbSize);
						free(pbData);
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
			pkts.push_back(pkt);

			if(ents.size() == 0)
				break;
		}
	}

	if(pReq->pcDirectory == LastGetDir && pkts.size() > 0) {
		if(pktNum == 0)
			printf("Reading directory \"%s\"...\n", pReq->pcDirectory);
		int n;
		FSPRequest* pSndReq = FSPRequest::Create(pReq->pHdr->command, (pkts[pktNum].data() + pktOff), (pkts[pktNum].size() - pktOff), pReq->pHdr->position, pReq->pHdr->sequence);
		UINT32 sndSize;
		PBYTE pbSndBuf = pSndReq->Pack(&sndSize);
		Utils::PrintHex(pbSndBuf, sndSize);
		if((n = sendto(srvSock, pbSndBuf, sndSize, 0, cliAddr, cliAddrLen)) == -1) {

		}
		free(pbSndBuf);
		delete pSndReq;
	}
}

void HandleBye(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen, FSPRequest* pReq) {
	int n;
	FSPRequest* pSndReq = FSPRequest::Create(CC_BYE, NULL, 0, pReq->pHdr->position, pReq->pHdr->sequence);
	UINT32 sndSize;
	PBYTE sndBuf = pSndReq->Pack(&sndSize);
	if((n = sendto(srvSock, sndBuf, sndSize, 0, cliAddr, cliAddrLen)) == -1) {

	}
	free(sndBuf);
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
		BYTE rcvBuf[FSP_MAXSPACE];
		memset(rcvBuf, 0, FSP_MAXSPACE);

		sockaddr_in cliAddr;
		memset(&cliAddr, 0, sizeof(cliAddr));
		if((n = recvfrom(srvSock, rcvBuf, FSP_MAXSPACE, 0, (sockaddr*)&cliAddr, (socklen_t*)&len)) == -1) {
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
		FSPRequest* pReq = FSPRequest::Parse(rcvBuf, n);

		printf("Command: %02X\n", pReq->pHdr->command);
		Utils::PrintHex(rcvBuf, n);

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
