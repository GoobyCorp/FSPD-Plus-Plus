#include "stdafx.hpp"

ulong Utils::AddressToLong(const PCHAR addr) {
    int a0, a1, a2, a3;
    sscanf(addr, "%i.%i.%i.%i", &a0, &a1, &a2, &a3);
    // bounds checks
    if(a0 >= 0 && a0 <= 255)
        if(a1 >= 0 && a1 <= 255)
            if(a2 >= 0 && a2 <= 255)
                if(a3 >= 0 && a3 <= 255) {
                    BYTE b[] = { (BYTE)a3, (BYTE)a2, (BYTE)a1, (BYTE)a0 };
                    return *(int*)&b;
                }
    return 0;
}

BYTE Utils::CalcClientToServerChecksum(PBYTE data, UINT32 size) {
    // null checksum
    *(PBYTE)(data + OFFS_CKSM) = 0;
    BYTE cksm = 0;
    for(int i = 0; i < size; i++)
        cksm += data[i];
    cksm += size;
    cksm += cksm >> 8;
    return cksm;
}

BYTE Utils::CalcServerToClientChecksum(PBYTE data, UINT32 size) {
    // set checksum to the size
    *(PBYTE)(data + OFFS_CKSM) = (BYTE)size;
    BYTE cksm = -size;
    for(int i = 0; i < size; i++)
        cksm += data[i];
    cksm += cksm >> 8;
    return cksm;
}