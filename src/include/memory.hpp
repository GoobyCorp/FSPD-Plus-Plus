#pragma once

class Alloc {
    public:
        PFSP_ALLOC pAlloc;

        Alloc(UINT32 size);
        ~Alloc();
        PVOID GetAddr();
        UINT32 GetSize();
        VOID Realloc(UINT32 size);
        VOID CopyTo(PVOID pvSrcAddr, UINT32 size, UINT32 offset = 0);
        VOID CopyFrom(PVOID pvDstAddr, UINT32 size, UINT32 offset = 0);
        VOID Free();
};