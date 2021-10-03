#include "stdafx.hpp"

RDIRENT::RDIRENT() {

}

RDIRENT::~RDIRENT() {
	if(this->pHdr != NULL)
		free(this->pHdr);
	if(this->pcFileName != NULL)
		free(this->pcFileName);
	this->pHdr = 0;
	this->pcFileName = 0;
}

UINT32 RDIRENT::GetSize() {
	PFSP_ALLOC pPck = this->Pack();
	free(pPck->pbData);  // quite a waste but whatever...
	return pPck->cbData;
}

PFSP_ALLOC RDIRENT::Pack() {
	UINT32 cbOut = 0;
	PBYTE pbOut = (PBYTE)calloc(1, FSP_SPACE);
	PBYTE pbTmp = pbOut;
	memcpy(pbTmp, &this->pHdr->FileTime, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->FileSize, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->Type, sizeof(BYTE));
	pbTmp += sizeof(BYTE);
	cbOut += (sizeof(UINT32) * 2) + sizeof(BYTE);
	Utils::SwapRDIRENTHeaderEndian((PRDIRENT_HDR)pbOut); // to big endian
	if(this->pcFileName != NULL) {
		strcpy((PCHAR)pbTmp, this->pcFileName);
		pbTmp += strlen(this->pcFileName) + 1;
		cbOut += strlen(this->pcFileName) + 1;
	}
	UINT32 padSize = Utils::CalcPadSize(cbOut, 4);
	memset(pbTmp, 0, padSize);
	cbOut += padSize;
	pbOut = (PBYTE)realloc(pbOut, cbOut);
	PFSP_ALLOC pAlloc = new FSP_ALLOC();
	pAlloc->pbData = pbOut;
	pAlloc->cbData = cbOut;
	return pAlloc;
}

RDIRENT* RDIRENT::Create(PCHAR path) {
	RDIRENT* pEnt = new RDIRENT();
	PRDIRENT_HDR pHdr = new RDIRENT_HDR();
	pHdr->FileTime = 1592534256;
	if(fs::is_regular_file(path)) {
		pHdr->FileSize = fs::file_size(path);
		pHdr->Type = RDTYPE_FILE;
		pEnt->pcFileName = Utils::AllocAndCopyString((PCHAR)fs::path(path).filename().c_str());
	} else if(fs::is_directory(path)) {
		pHdr->FileSize = 0;
		pHdr->Type = RDTYPE_DIR;
		pEnt->pcFileName = Utils::AllocAndCopyString((PCHAR)fs::path(path).filename().c_str());
	}
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(pHdr, sizeof(RDIRENT_HDR));
	delete pHdr;
	return pEnt;
}

RDIRENT* RDIRENT::CreateSkip() {
	RDIRENT* pEnt = new RDIRENT();
	PRDIRENT_HDR pHdr = new RDIRENT_HDR();
	pHdr->FileTime = 0;
	pHdr->FileSize = 0;
	pHdr->Type = RDTYPE_SKIP;
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(pHdr, sizeof(RDIRENT_HDR));
	delete pHdr;
	return pEnt;
}

RDIRENT* RDIRENT::CreateEnd() {
	RDIRENT* pEnt = new RDIRENT();
	PRDIRENT_HDR pHdr = new RDIRENT_HDR();
	pHdr->FileTime = 0;
	pHdr->FileSize = 0;
	pHdr->Type = RDTYPE_END;
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(pHdr, sizeof(RDIRENT_HDR));
	delete pHdr;
	return pEnt;
}

STAT::STAT() {

}

STAT::~STAT() {
	if(this->pHdr != NULL)
		free(this->pHdr);
	this->pHdr = 0;
}

UINT32 STAT::GetSize() {
	PFSP_ALLOC pPck = this->Pack();
	free(pPck->pbData);  // quite a waste but whatever...
	return pPck->cbData;
}

PFSP_ALLOC STAT::Pack() {
	UINT32 cbOut = 0;
	PBYTE pbOut = (PBYTE)calloc(1, FSP_SPACE);
	PBYTE pbTmp = pbOut;
	memcpy(pbTmp, &this->pHdr->FileTime, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->FileSize, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->Type, sizeof(BYTE));
	pbTmp += sizeof(BYTE);
	cbOut += (sizeof(UINT32) * 2) + sizeof(BYTE);
	Utils::SwapSTATHeaderEndian((PSTAT_HDR)pbOut); // to big endian
	pbOut = (PBYTE)realloc(pbOut, cbOut);
	PFSP_ALLOC pAlloc = new FSP_ALLOC();
	pAlloc->pbData = pbOut;
	pAlloc->cbData = cbOut;
	return pAlloc;
}

STAT* STAT::Create(PCHAR path) {
	STAT* pEnt = new STAT();
	PSTAT_HDR pHdr = new STAT_HDR();
	pHdr->FileTime = 1592534256;
	if(fs::is_regular_file(path)) {
		pHdr->FileSize = fs::file_size(path);
		pHdr->Type = RDTYPE_FILE;
	} else if(fs::is_directory(path)) {
		pHdr->FileSize = 0;
		pHdr->Type = RDTYPE_DIR;
	} else {
		pHdr->FileSize = 0;
		pHdr->Type = 0;
	}
	pEnt->pHdr = (PSTAT_HDR)Utils::AllocAndCopy(pHdr, sizeof(STAT));
	delete pHdr;
	return pEnt;
}

FSPRequest::FSPRequest() {

}

FSPRequest::~FSPRequest() {
	if(this->pHdr != NULL)
		free(this->pHdr);
	if(this->pbData != NULL)
	    free(this->pbData);
	if(this->pbExtra != NULL)
		free(this->pbExtra);
	if(this->pcDirectory != NULL)
		free(this->pcDirectory);
	if(this->pcFilename != NULL)
		free(this->pcFilename);
	if(this->pcPassword != NULL)
		free(this->pcPassword);
	this->pHdr = 0;
	this->pbData = 0;
	this->pbExtra = 0;
	this->pcDirectory = 0;
	this->pcFilename = 0;
	this->pcPassword = 0;
}

UINT32 FSPRequest::GetSize() {
	return sizeof(FSP_HDR) + this->cbData + this->cbExtra;
}

PFSP_ALLOC FSPRequest::Pack() {
	PBYTE pbOut = (PBYTE)calloc(1, FSP_MAXSPACE);
	memcpy(pbOut, this->pHdr, sizeof(FSP_HDR));
	Utils::SwapFSPHeaderEndian((PFSP_HDR)pbOut);  // to big endian
	if(this->pbData != NULL && this->cbData > 0)
		memcpy(pbOut + sizeof(FSP_HDR), this->pbData, this->cbData);
	if(this->pbExtra != NULL && this->cbExtra > 0)
		memcpy(pbOut + sizeof(FSP_HDR) + this->cbData, this->pbExtra, this->cbExtra);
	int cbOut = this->GetSize();
	pbOut = (PBYTE)realloc(pbOut, cbOut);
	pbOut[OFFS_CKSM] = Utils::CalcServerToClientChecksum(pbOut, cbOut);
	PFSP_ALLOC pAlloc = new FSP_ALLOC();
	pAlloc->pbData = pbOut;
	pAlloc->cbData = cbOut;
	return pAlloc;
}

FSPRequest* FSPRequest::Parse(PBYTE pbData, UINT32 cbData) {
	FSPRequest* pReq = new FSPRequest();
	pReq->pHdr = (PFSP_HDR)Utils::AllocAndCopy(pbData, sizeof(FSP_HDR));

	Utils::SwapFSPHeaderEndian(pReq->pHdr);  // to little endian

	pReq->cbData = pReq->pHdr->length;
	pReq->cbExtra = cbData - (sizeof(FSP_HDR) + pReq->pHdr->length);

	pReq->pbData = (PBYTE)Utils::AllocAndCopy(pbData + sizeof(FSP_HDR), pReq->cbData);
	pReq->pbExtra = (PBYTE)Utils::AllocAndCopy(pbData + sizeof(FSP_HDR) + pReq->pHdr->length, pReq->cbExtra);

	BYTE storCksm = pReq->pHdr->checksum;
	BYTE calcCksm = Utils::CalcClientToServerChecksum(pbData, pReq->GetSize());
	if(storCksm != calcCksm) {
		printf("Checksum mismatch!\n");
	}

	if(pReq->pHdr->command == CC_GET_DIR) {
		string s((PCHAR)pReq->pbData, pReq->cbData);
		int pos;
		if((pos = s.find("\n")) < string::npos) {
			pReq->pcDirectory = Utils::AllocAndCopyString((PCHAR)s.substr(0, pos).c_str());
			pReq->pcPassword = Utils::AllocAndCopyString((PCHAR)s.substr(pos + 1).c_str());
		}
		if(pReq->cbExtra == 2) {
			pReq->blkSize = bswap16(*(PUINT16)pReq->pbExtra);
		}
	} else if(pReq->pHdr->command == CC_GET_FILE ||
		pReq->pHdr->command == CC_STAT ||
		pReq->pHdr->command == CC_DEL_FILE ||
		pReq->pHdr->command == CC_INSTALL) {

		string s((PCHAR)pReq->pbData, pReq->cbData);
		int pos;
		if((pos = s.find("\n")) < string::npos) {
			pReq->pcFilename = Utils::AllocAndCopyString((PCHAR)s.substr(0, pos).c_str());
			pReq->pcPassword = Utils::AllocAndCopyString((PCHAR)s.substr(pos + 1).c_str());
		}
		if(pReq->pHdr->command == CC_GET_FILE && pReq->cbExtra == 2) {
			pReq->blkSize = bswap16(*(PUINT16)pReq->pbExtra);
		}
	}

	return pReq;
}

FSPRequest* FSPRequest::Create(BYTE cmd, PBYTE pbData, UINT16 cbData, PBYTE pbExtra, UINT16 cbExtra, UINT16 pos, UINT16 seq) {
	FSPRequest* pReq = new FSPRequest();

	PFSP_HDR pHdr = new FSP_HDR();

	pHdr->command = cmd;
	if(Globals::FSP_KEY == 0)
		pHdr->key = randint(0x1111, 0xFFFF);
	else
		pHdr->key = Globals::FSP_KEY;
	pHdr->sequence = seq;
	pHdr->length = cbData;
	pHdr->position = pos;
	pHdr->checksum = 0;

	pReq->pHdr = (PFSP_HDR)Utils::AllocAndCopy(pHdr, sizeof(FSP_HDR));
	if(pbData != NULL && cbData > 0) {
		pReq->pbData = (PBYTE)Utils::AllocAndCopy(pbData, cbData);
		pReq->cbData = cbData;
	}
	if(pbExtra != NULL && cbExtra > 0) {
		pReq->pbExtra = (PBYTE)Utils::AllocAndCopy(pbExtra, cbExtra);
		pReq->cbExtra = cbExtra;
	}

	if(Globals::FSP_KEY == 0)
		Globals::FSP_KEY = pHdr->key;
	delete pHdr;

	return pReq;
}