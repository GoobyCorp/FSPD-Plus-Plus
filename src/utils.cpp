#include "stdafx.hpp"

ULONG Utils::AddressToLong(const PCHAR addr) {
    int a0, a1, a2, a3;
    sscanf(addr, "%i.%i.%i.%i", &a0, &a1, &a2, &a3);
    // bounds checks
    if(a0 >= 0 && a0 <= 255)
        if(a1 >= 0 && a1 <= 255)
            if(a2 >= 0 && a2 <= 255)
                if(a3 >= 0 && a3 <= 255) {
                    BYTE b[] = { a0, a1, a2, a3 };
                    return *(PULONG)&b;
                }
    return 0;
}

void Utils::LongToAddress(PCHAR addrStr, ULONG addrLong) {
    PBYTE pbAddr = (PBYTE)&addrLong;
    sprintf(addrStr, "%i.%i.%i.%i", *pbAddr, *(pbAddr + 1), *(pbAddr + 2), *(pbAddr + 3));
}

void Utils::PrintHex(PBYTE data, UINT32 size) {
    for(int i = 0; i < size; i++)
        printf("%02X", data[i]);
    printf("\n");
}

int Utils::ByteSum(PBYTE data, UINT32 size) {
    int sum = 0;
    for(int i = 0; i < size; i++)
        sum += data[i];
    return sum;
}

BYTE Utils::CalcClientToServerChecksum(PBYTE data, UINT32 size) {
    // null checksum
    *(PBYTE)(data + OFFS_CKSM) = 0;

    UINT32 sum = size;
    UINT32 cksm;
    PBYTE t;
    for(size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    cksm = sum + (sum >> 8);
    return cksm;
}

BYTE Utils::CalcServerToClientChecksum(PBYTE data, UINT32 size) {
    // null checksum
    *(PBYTE)(data + OFFS_CKSM) = 0;

    int sum = 0;
    UINT32 cksm;
    PBYTE t;
    for(size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    cksm = sum + (sum >> 8);
    return cksm;
}

UINT32 Utils::CalcPadSize(UINT32 size, UINT32 boundary) {
    if(size == boundary)
        return 0;
    else
        return (boundary - size % boundary);
}

BOOL Utils::IsDir(PCHAR path) {
    return fs::is_directory(path);
}

BOOL Utils::IsFile(PCHAR path) {
    return fs::is_regular_file(path);
}

PVOID Utils::AllocAndCopy(PVOID pData, UINT32 size) {
    void* t = calloc(1, size);
    memcpy(t, pData, size);
    return t;
}

PCHAR Utils::AllocAndCopyString(PCHAR str) {
    PCHAR t = (PCHAR)calloc(1, strlen(str) + 1);
    memcpy(t, str, strlen(str));
    return t;
}

VOID Utils::CopyToVector(vector<BYTE>* pbVec, PBYTE pbData, UINT32 cbData) {
    for(int i = 0; i < cbData; i++) {
        pbVec->push_back(pbData[i]);
    }
}

VOID Utils::ClearVector(vector<vector<BYTE>>* pbVec) {
    for(int i = 0; i < pbVec->size(); i++) {
        pbVec[i].clear();
    }
    pbVec->clear();
}

PCHAR Utils::StripDirSep(PCHAR path) {
    string s(path);
    return path + s.find_first_not_of('/');
}

VOID Utils::SwapFSPHeaderEndian(PFSP_HDR pHdr) {
    es16(pHdr->key);
    es16(pHdr->sequence);
    es16(pHdr->length);
    es32(pHdr->position);
}

VOID Utils::SwapRDIRENTHeaderEndian(PRDIRENT_HDR pHdr) {
    es32(pHdr->FileTime);
    es32(pHdr->FileSize);
}

VOID Utils::SwapSTATHeaderEndian(PSTAT_HDR pHdr) {
    es32(pHdr->FileTime);
    es32(pHdr->FileSize);
}