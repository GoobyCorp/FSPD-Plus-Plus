#include "stdafx.hpp"

RDIRENT::RDIRENT() {

}

RDIRENT::~RDIRENT() {
    
}

UINT32 RDIRENT::GetSize() {
    BYTE pbOut[PATH_MAX];
    return this->Pack(pbOut);
}

UINT32 RDIRENT::Pack(PBYTE pbOut) {
    UINT32 size = 0;
    memcpy(pbOut, &this->FileTime, sizeof(UINT32));
    pbOut += sizeof(UINT32);
    memcpy(pbOut, &this->FileSize, sizeof(UINT32));
    pbOut += sizeof(UINT32);
    memcpy(pbOut, &this->Type, sizeof(BYTE));
    pbOut += sizeof(BYTE);
    size += (sizeof(UINT32) * 2) + sizeof(BYTE);
    memcpy(pbOut, this->pcFileName, strlen(this->pcFileName));
    pbOut += strlen(this->pcFileName);
    size += strlen(this->pcFileName);
    UINT32 padSize = Utils::CalcPadSize(size, 4);
    memset(pbOut, 0, padSize);
    size += padSize;
    return size;
}

RDIRENT RDIRENT::Create(PCHAR path) {
    fs::path p(path);
    RDIRENT ent;
    ent.FileTime = 1592534256;
    if(fs::is_regular_file(p)) {
        ent.FileSize = fs::file_size(p);
        ent.Type = RDTYPE_FILE;
        ent.pcFileName = (PCHAR)p.filename().c_str();
    } else if(fs::is_directory(p)) {
        ent.FileSize = 0;
        ent.Type = RDTYPE_DIR;
        ent.pcFileName = (PCHAR)p.filename().c_str();
    }
    return ent;
}

RDIRENT RDIRENT::CreateSkip() {
    RDIRENT ent;
    ent.Type = RDTYPE_SKIP;
    return ent;
}

RDIRENT RDIRENT::CreateEnd() {
    RDIRENT ent;
    ent.Type = RDTYPE_END;
    return ent;
}

FSPRequest::FSPRequest() {

}

FSPRequest::~FSPRequest() {

}

UINT32 FSPRequest::GetSize() {
    return sizeof(FSP_HDR) + this->cbData + this->cbExtra;
}

UINT32 FSPRequest::Pack(PBYTE pbOut) {
    memcpy(pbOut, this->pHdr, sizeof(FSP_HDR));
    Utils::SwapFSPHeaderEndian((PFSP_HDR)pbOut);  // to big endian
    memcpy(pbOut + sizeof(FSP_HDR), this->pbData, this->cbData);
    memcpy(pbOut + sizeof(FSP_HDR) + this->cbData, this->pbExtra, this->cbExtra);
    return this->GetSize();
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
                req.pcDirectory = (PCHAR)s.substr(0, pos).c_str();
                req.pcPassword = (PCHAR)s.substr(pos + 1).c_str();
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
                req.pcFilename = (PCHAR)s.substr(0, pos).c_str();
                req.pcPassword = (PCHAR)s.substr(pos + 1).c_str();
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
    PBYTE packed = (PBYTE)malloc(FSP_MAXSPACE);
    USHORT cbOut = req.Pack(packed);
    packed = (PBYTE)realloc(packed, cbOut);
    hdr.checksum = Utils::CalcServerToClientChecksum(packed, cbOut);
    free(packed);

    return req;
}