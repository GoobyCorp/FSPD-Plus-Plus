#include "stdafx.hpp"

RDIRENT::RDIRENT() {

}

RDIRENT::~RDIRENT() {
    if(this->pcFileName != NULL)
        free(this->pcFileName);
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

RDIRENT RDIRENT::Create(PCHAR path) {
    fs::path p(path);
    RDIRENT ent;
    RDIRENT_HDR hdr;
    hdr.FileTime = 1592534256;
    if(fs::is_regular_file(p)) {
        hdr.FileSize = fs::file_size(p);
        hdr.Type = RDTYPE_FILE;
        PCHAR fName = (PCHAR)p.filename().c_str();
        ent.pcFileName = (PCHAR)malloc(strlen(fName) + 1);
        strcpy(ent.pcFileName, fName);
    } else if(fs::is_directory(p)) {
        hdr.FileSize = 0;
        hdr.Type = RDTYPE_DIR;
        PCHAR fName = (PCHAR)p.filename().c_str();
        ent.pcFileName = (PCHAR)malloc(strlen(fName) + 1);
        strcpy(ent.pcFileName, fName);
    }
    ent.pHdr = &hdr;
    return ent;
}

RDIRENT RDIRENT::CreateSkip() {
    RDIRENT ent;
    RDIRENT_HDR hdr;
    hdr.Type = RDTYPE_SKIP;
    ent.pHdr = &hdr;
    return ent;
}

RDIRENT RDIRENT::CreateEnd() {
    RDIRENT ent;
    RDIRENT_HDR hdr;
    hdr.Type = RDTYPE_END;
    ent.pHdr = &hdr;
    return ent;
}

FSPRequest::FSPRequest() {

}

FSPRequest::~FSPRequest() {
    if(this->pcDirectory != NULL)
        free(this->pcDirectory);
    if(this->pcFilename != NULL)
        free(this->pcFilename);
    if(this->pcPassword != NULL)
        free(this->pcPassword);
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
    return pbOut;
}

FSPRequest FSPRequest::Parse(PBYTE pbData, UINT32 cbData) {
    FSPRequest req;
    req.pHdr = (PFSP_HDR)pbData;
    Utils::SwapFSPHeaderEndian(req.pHdr);  // to little endian
    req.pbData = pbData + sizeof(FSP_HDR);
    req.pbExtra = pbData + sizeof(FSP_HDR) + req.pHdr->length;
    req.cbData = req.pHdr->length;
    req.cbExtra = cbData - (sizeof(FSP_HDR) + req.pHdr->length);

    BYTE storCksm = req.pHdr->checksum;
    BYTE calcCksm = Utils::CalcClientToServerChecksum(pbData, req.GetSize());
    if(storCksm != calcCksm) {
        printf("Checksum mismatch!\n");
    }

    switch(req.pHdr->command) {
        case CC_GET_DIR: {
            string s((PCHAR)req.pbData, req.cbData);
            int pos;
            if((pos = s.find("\n")) < string::npos) {
                PCHAR dName = (PCHAR)s.substr(0, pos).c_str();
                req.pcDirectory = (PCHAR)calloc(1, strlen(dName) + 1);
                strcpy(req.pcDirectory, dName);
                
                PCHAR pWord = (PCHAR)s.substr(pos + 1).c_str();
                req.pcPassword = (PCHAR)calloc(1, strlen(pWord) + 1);
                strcpy(req.pcPassword, pWord);
            }
            break;
        }
        case CC_GET_FILE:
        case CC_STAT:
        case CC_DEL_FILE:
        case CC_INSTALL: {
            string s((PCHAR)req.pbData, req.cbData);
            int pos;
            if((pos = s.find("\n")) < string::npos) {
                PCHAR fName = (PCHAR)s.substr(0, pos).c_str();
                req.pcFilename = (PCHAR)calloc(1, strlen(fName) + 1);
                strcpy(req.pcFilename, fName);
                
                PCHAR pWord = (PCHAR)s.substr(pos + 1).c_str();
                req.pcPassword = (PCHAR)calloc(1, strlen(pWord) + 1);
                strcpy(req.pcPassword, pWord);
            }
            break;
        }
        default: {
            break;
        }
    }

    return req;
}

FSPRequest FSPRequest::Create(FSP_COMMAND cmd, PBYTE pbData, USHORT cbData, USHORT pos, USHORT seq) {
    FSPRequest req;
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
    
    req.pHdr = &hdr;
    req.pbData = pbData;
    req.cbData = cbData;

    if(Globals::FSP_KEY == 0)
        Globals::FSP_KEY = hdr.key;

    // pack the data so we can calculate the checksum
    // probably a shitty way to do it :'(
    UINT32 cbOut;
    PBYTE pbOut = req.Pack(&cbOut);
    hdr.checksum = Utils::CalcServerToClientChecksum(pbOut, cbOut);
    free(pbOut);

    return req;
}