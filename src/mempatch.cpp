//
// Created by Michal Přikryl on 22.04.2026.
// Copyright (c) 2026 slynxcz. All rights reserved.
//
#include "mempatch.h"

#include <ISmmPlugin.h>

#include "plat.h"
#include "tier0/dbg.h"
#include "tier1/strtools.h"
#include "memaddr.hpp"
#include "module.hpp"
#include "plugin.h"

#include "tier0/memdbgon.h"

bool CMemPatch::PerformPatch(DynLibUtils::CMemory addr,
                             const uint8_t* pPatchBytes,
                             size_t patchLen,
                             int iOffset)
{
    // already patched
    if (m_pOriginalBytes)
        return true;

    uint8_t* pAddr = addr.RCast<uint8_t*>() + iOffset;

    m_pPatchAddress = (uintptr_t)pAddr;
    m_iPatchLength = patchLen;

    // backup original bytes
    m_pOriginalBytes = new byte[m_iPatchLength];
    V_memcpy(m_pOriginalBytes, (void*)m_pPatchAddress, m_iPatchLength);

    // apply patch (Plat handles protection internally)
    Plat_WriteMemory((void*)m_pPatchAddress, (byte*)pPatchBytes, m_iPatchLength);

    META_LOG(&g_Plugin, "Patched %s at %p\n", m_pszName, (void*)m_pPatchAddress);
    return true;
}

void CMemPatch::UndoPatch()
{
    if (!m_pOriginalBytes)
        return;

    META_LOG(&g_Plugin, "Undoing patch %s at %p\n", m_pszName, m_pPatchAddress);

    Plat_WriteMemory((void*)m_pPatchAddress, m_pOriginalBytes, m_iPatchLength);

    delete[] m_pOriginalBytes;
    m_pOriginalBytes = nullptr;
}