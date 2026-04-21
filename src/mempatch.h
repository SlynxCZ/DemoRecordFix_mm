//
// Created by Michal Přikryl on 22.04.2026.
// Copyright (c) 2026 slynxcz. All rights reserved.
//
#ifndef MEMPATCH_H
#define MEMPATCH_H
#ifdef _WIN32
#pragma once
#endif
#include "memaddr.hpp"
#include "ISmmAPI.h"

class CMemPatch
{
public:
    CMemPatch(const char* pszName, const char* pOffsetName = "") :
         m_pszName(pszName)
    {
        m_pPatchAddress = 0x00;
        m_pOriginalBytes = nullptr;
        m_pPatch = nullptr;
        m_iPatchLength = 0;
    }

    bool PerformPatch(DynLibUtils::CMemory addr, const uint8_t* pPatchBytes, size_t patchLen, int iOffset);
    void UndoPatch();

    uintptr_t GetPatchAddress() { return m_pPatchAddress; }

private:
    const byte* m_pPatch;
    byte* m_pOriginalBytes;
    const char* m_pszName;
    size_t m_iPatchLength;
    uintptr_t m_pPatchAddress;
};

#endif //MEMPATCH_H
