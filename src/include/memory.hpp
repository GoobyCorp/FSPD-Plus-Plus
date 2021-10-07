#pragma once

class Alloc {
    public:
        Alloc(UINT32 cbSize);
        Alloc(PBYTE pbData, UINT32 cbSize);
        ~Alloc();
        PVOID GetAddr();
        UINT32 GetSize();
        VOID Realloc(UINT32 size);
        VOID CopyTo(PVOID pvSrcAddr, UINT32 size, UINT32 dstOffs = 0);
        VOID CopyFrom(PVOID pvDstAddr, UINT32 size, UINT32 srcOffs = 0);
        VOID Free();
    private:
        PFSP_ALLOC pAlloc;
};