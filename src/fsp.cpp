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
	UINT32 size;
	PBYTE pbOut = this->Pack(&size);
	free(pbOut);  // quite a waste but whatever...
	return size;
}

PBYTE RDIRENT::Pack(PUINT32 pcbOut) {
	UINT32 size = 0;
	PBYTE pbOut = (PBYTE)calloc(1, FSP_MAXSPACE);
	PBYTE pbTmp = pbOut;
	memcpy(pbTmp, &this->pHdr->FileTime, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->FileSize, sizeof(UINT32));
	pbTmp += sizeof(UINT32);
	memcpy(pbTmp, &this->pHdr->Type, sizeof(BYTE));
	pbTmp += sizeof(BYTE);
	size += (sizeof(UINT32) * 2) + sizeof(BYTE);
	Utils::SwapRDIRENTHeaderEndian((PRDIRENT_HDR)pbOut); // to big endian
	if(this->pcFileName != NULL) {
		strcpy((PCHAR)pbTmp, this->pcFileName);
		pbTmp += strlen(this->pcFileName) + 1;
		size += strlen(this->pcFileName) + 1;
	}
	UINT32 padSize = Utils::CalcPadSize(size, 4);
	memset(pbTmp, 0, padSize);
	size += padSize;
	*pcbOut = size;
	pbOut = (PBYTE)realloc(pbOut, size);
	return pbOut;
}

RDIRENT* RDIRENT::Create(PCHAR path) {
	fs::path p(Globals::FSP_DIRECTORY);
	p /= path;
	RDIRENT* pEnt = new RDIRENT();
	RDIRENT_HDR hdr;
	hdr.FileTime = 1592534256;
	if(fs::is_regular_file(p)) {
		hdr.FileSize = fs::file_size(p);
		hdr.Type = RDTYPE_FILE;
		pEnt->pcFileName = Utils::AllocAndCopyString((PCHAR)p.filename().c_str());
	} else if(fs::is_directory(p)) {
		hdr.FileSize = 0;
		hdr.Type = RDTYPE_DIR;
		pEnt->pcFileName = Utils::AllocAndCopyString((PCHAR)p.filename().c_str());
	}
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(&hdr, sizeof(RDIRENT_HDR));
	return pEnt;
}

RDIRENT* RDIRENT::CreateSkip() {
	RDIRENT* pEnt = new RDIRENT();
	RDIRENT_HDR hdr;
	hdr.Type = RDTYPE_SKIP;
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(&hdr, sizeof(RDIRENT_HDR));
	return pEnt;
}

RDIRENT* RDIRENT::CreateEnd() {
	RDIRENT* pEnt = new RDIRENT();
	RDIRENT_HDR hdr;
	hdr.Type = RDTYPE_END;
	pEnt->pHdr = (PRDIRENT_HDR)Utils::AllocAndCopy(&hdr, sizeof(RDIRENT_HDR));
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

PBYTE FSPRequest::Pack(PUINT32 pcbOut) {
	PBYTE pbOut = (PBYTE)calloc(1, FSP_MAXSPACE);
	memcpy(pbOut, this->pHdr, sizeof(FSP_HDR));
	Utils::SwapFSPHeaderEndian((PFSP_HDR)pbOut);  // to big endian
	if(this->cbData > 0)
		memcpy(pbOut + sizeof(FSP_HDR), this->pbData, this->cbData);
	if(this->cbExtra > 0)
		memcpy(pbOut + sizeof(FSP_HDR) + this->cbData, this->pbExtra, this->cbExtra);
	*pcbOut = this->GetSize();
	pbOut = (PBYTE)realloc(pbOut, *pcbOut);
	pbOut[OFFS_CKSM] = Utils::CalcServerToClientChecksum(pbOut, *pcbOut);
	return pbOut;
}

FSPRequest* FSPRequest::Parse(PBYTE pbData, UINT32 cbData) {
	FSPRequest* req = new FSPRequest();
	req->pHdr = (PFSP_HDR)Utils::AllocAndCopy(pbData, sizeof(FSP_HDR));

	Utils::SwapFSPHeaderEndian(req->pHdr);  // to little endian

	req->cbData = req->pHdr->length;
	req->cbExtra = cbData - (sizeof(FSP_HDR) + req->pHdr->length);

	req->pbData = (PBYTE)Utils::AllocAndCopy(pbData + sizeof(FSP_HDR), req->cbData);
	req->pbExtra = (PBYTE)Utils::AllocAndCopy(pbData + sizeof(FSP_HDR) + req->pHdr->length, req->cbExtra);

	BYTE storCksm = req->pHdr->checksum;
	BYTE calcCksm = Utils::CalcClientToServerChecksum(pbData, req->GetSize());
	if(storCksm != calcCksm) {
		printf("Checksum mismatch!\n");
	}

	switch(req->pHdr->command) {
		case CC_GET_DIR: {
			string s((PCHAR)req->pbData, req->cbData);
			int pos;
			if((pos = s.find("\n")) < string::npos) {
				req->pcDirectory = Utils::AllocAndCopyString((PCHAR)s.substr(0, pos).c_str());
				req->pcPassword = Utils::AllocAndCopyString((PCHAR)s.substr(pos + 1).c_str());
			}
			break;
		}
		case CC_GET_FILE:
		case CC_STAT:
		case CC_DEL_FILE:
		case CC_INSTALL: {
			string s((PCHAR)req->pbData, req->cbData);
			int pos;
			if((pos = s.find("\n")) < string::npos) {
				req->pcFilename = Utils::AllocAndCopyString((PCHAR)s.substr(0, pos).c_str());
				req->pcPassword = Utils::AllocAndCopyString((PCHAR)s.substr(pos + 1).c_str());
			}
			break;
		}
		default: {
			break;
		}
	}

	return req;
}

FSPRequest* FSPRequest::Create(BYTE cmd, PBYTE pbData, UINT16 cbData, UINT16 pos, UINT16 seq) {
	FSPRequest* req = new FSPRequest();

	FSP_HDR hdr;
	memset(&hdr, 0, sizeof(FSP_HDR));

	hdr.command = cmd;
	if(Globals::FSP_KEY == 0)
		hdr.key = randint(0x1000, 0xFFFF);
	else
		hdr.key = Globals::FSP_KEY;
	hdr.sequence = seq;
	hdr.length = cbData;
	hdr.position = pos;

	req->pHdr = (PFSP_HDR)Utils::AllocAndCopy(&hdr, sizeof(FSP_HDR));

	req->pbData = (PBYTE)Utils::AllocAndCopy(pbData, cbData);
	req->cbData = cbData;

	if(Globals::FSP_KEY == 0)
		Globals::FSP_KEY = hdr.key;

	hdr.checksum = 0;

	// pack the data so we can calculate the checksum
	// probably a shitty way to do it :'(
	// UINT32 cbOut;
	// PBYTE pbOut = req->Pack(&cbOut);
	// hdr.checksum = Utils::CalcServerToClientChecksum(pbOut, cbOut);
	// free(pbOut);

	return req;
}