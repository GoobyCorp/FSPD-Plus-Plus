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

BYTE Utils::ComputeChecksum(PBYTE data, UINT32 size, UINT32 sum) {
    // null checksum
    *(PBYTE)(data + OFFS_CKSM) = 0;

    UINT32 cksm;
    PBYTE t;
    for(t = data; t < (data + size); sum += *t++);
    cksm = sum + (sum >> 8);
    return cksm;
}

BYTE Utils::CalcClientToServerChecksum(PBYTE data, UINT32 size) {
    return Utils::ComputeChecksum(data, size, size);
}

BYTE Utils::CalcServerToClientChecksum(PBYTE data, UINT32 size) {
    return Utils::ComputeChecksum(data, size, 0);
}

void Utils::SwapFSPHeaderEndian(PFSP_HDR pHdr) {
    es16(pHdr->key);
    es16(pHdr->sequence);
    es16(pHdr->length);
    es32(pHdr->position);
}