#include "stdafx.hpp"

FSPRequest::FSPRequest() {

}

FSPRequest::~FSPRequest() {

}

USHORT FSPRequest::GetSize() {
    return sizeof(FSP_HDR) + this->cbData + this->cbExtra;
}

USHORT FSPRequest::Serialize(PBYTE pbOut) {
    memcpy(pbOut, this->pHdr, sizeof(FSP_HDR));
    memcpy(pbOut + sizeof(FSP_HDR), this->pbData, this->cbData);
    memcpy(pbOut + sizeof(FSP_HDR) + this->cbData, this->pbExtra, this->cbExtra);
    return this->GetSize();
}

FSPRequest FSPRequest::Parse(PBYTE pbData, UINT32 cbData) {
    FSPRequest req;
    req.pHdr = (PFSP_HDR)pbData;
    req.pbData = pbData + sizeof(FSP_HDR);
    req.pbExtra = pbData + sizeof(FSP_HDR) + req.pHdr->length;
    req.cbData = req.pHdr->length;
    req.cbExtra = cbData - (sizeof(FSP_HDR) + req.pHdr->length);

    BYTE storCksm = req.pHdr->checksum;
    BYTE calcCksm = Utils::CalcClientToServerChecksum(pbData, sizeof(FSP_HDR) + req.pHdr->length);
    if(storCksm != calcCksm) {
        printf("Checksum mismatch!\n");
    }

    switch(req.pHdr->command) {
        case CC_GET_DIR: {
            break;
        }
        case CC_GET_FILE:
        case CC_STAT:
        case CC_DEL_FILE:
        case CC_INSTALL: {
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
    hdr.key = randint(0x1000, 0xFFFF);
    hdr.sequence = seq;
    hdr.length = cbData;
    hdr.position = pos;
    
    req.pHdr = &hdr;
    req.pbData = pbData;
    req.cbData = cbData;

    // pack the data so we can calculate the checksum
    // probably a shitty way to do it :'(
    PBYTE packed = (PBYTE)malloc(FSP_MAXSPACE);
    USHORT cbOut = req.Serialize(packed);
    packed = (PBYTE)realloc(packed, cbOut);
    hdr.checksum = Utils::CalcServerToClientChecksum(packed, cbOut);
    free(packed);

    return req;
}