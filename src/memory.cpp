#include "stdafx.hpp"

Alloc::Alloc(UINT32 size) {
    this->pAlloc = new FSP_ALLOC();
    this->pAlloc->pbData = (PBYTE)calloc(1, size);
    this->pAlloc->cbData = size;
}

Alloc::~Alloc() {
    this->Free();
    delete this->pAlloc;
}

PVOID Alloc::GetAddr() {
    return this->pAlloc->pbData;
}

UINT32 Alloc::GetSize() {
    return this->pAlloc->cbData;
}

VOID Alloc::Realloc(UINT32 size) {
    this->pAlloc->pbData = (PBYTE)realloc(this->pAlloc->pbData, size);
    this->pAlloc->cbData = size;
}

VOID Alloc::CopyTo(PVOID pvSrcAddr, UINT32 size, UINT32 offset) {
    memcpy(this->pAlloc->pbData + offset, pvSrcAddr, size);
}

VOID Alloc::CopyFrom(PVOID pvDstAddr, UINT32 size, UINT32 offset) {
    memcpy(pvDstAddr, this->pAlloc->pbData + offset, size);
}

VOID Alloc::Free() {
    free(this->pAlloc->pbData);
}