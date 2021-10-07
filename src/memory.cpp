#include "stdafx.hpp"

// create new allocation
Alloc::Alloc(UINT32 cbSize) {
    this->pAlloc = new FSP_ALLOC();
    this->pAlloc->pbData = (PBYTE)calloc(1, cbSize);
    this->pAlloc->cbData = cbSize;
}

// use preallocated memory
Alloc::Alloc(PBYTE pbData, UINT32 cbSize) {
    this->pAlloc = new FSP_ALLOC();
    this->pAlloc->pbData = pbData;
    this->pAlloc->cbData = cbSize;
}

Alloc::~Alloc() {
    this->Free();
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

VOID Alloc::CopyTo(PVOID pvSrcAddr, UINT32 size, UINT32 dstOffs) {
    memcpy(this->pAlloc->pbData + dstOffs, pvSrcAddr, size);
}

VOID Alloc::CopyFrom(PVOID pvDstAddr, UINT32 size, UINT32 srcOffs) {
    memcpy(pvDstAddr, this->pAlloc->pbData + srcOffs, size);
}

VOID Alloc::Free() {
    free(this->pAlloc->pbData);
    delete this->pAlloc;
}