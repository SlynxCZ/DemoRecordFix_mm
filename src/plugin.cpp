// Author: Michal Přikryl (Slynx) <github.com/SlynxCZ>

#include "plugin.h"
#include "utils.hpp"

#include "macros.h"
#include "memaddr.hpp"
#include "module.hpp"
#include "virtual.hpp"
#include "vthook.hpp"

#include "eiface.h"
#include "interfaces/interfaces.h"
#include "entitysystem.h"

#include <cstdint>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <unordered_set>

#define VERSION_STRING SEMVER " @ " GITHUB_SHA
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

using namespace DynLibUtils;

Plugin g_Plugin;
PLUGIN_EXPOSE(Plugin, g_Plugin);

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    if (late)
    {
        std::snprintf(error, maxlen, "Late load is not supported");
        return false;
    }

    PLUGIN_SAVEVARS();

    GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);

    CModule libserver(g_pSource2Server);

    // https://github.com/Source2ZE/CS2Fixes/commit/61937f78dd649ed391f6988b0c58ae4a75fd4bc6
    {
        CMemory pSetSchemaHammerUniqueId = libserver.FindPattern(ParseStringPattern(WIN_LINUX("75 ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 84 C0 74 ? 48 8D 05", "75 ? 48 8B 03 48 8D 15 ? ? ? ? 48 8B 80 ? ? ? ? 48 39 D0 75 ? 48 83 C4")));
        if (!pSetSchemaHammerUniqueId)
        {
            std::snprintf(error, maxlen, "Failed to find SetSchemaHammerUniqueId");
            return false;
        }

        uint8_t* p = pSetSchemaHammerUniqueId.RCast<uint8_t*>();
        if (*p != 0x75)
        {
            std::snprintf(error, maxlen, "Unexpected opcode at SetSchemaHammerUniqueId (expected 0x75, got 0x%02X)", *p);
            return false;
        }

        m_pPatchAddr = p;
        m_OriginalByte = *p;

        VirtualUnprotector unprotect(p, 1);
        *p = 0xEB;

        m_bPatched = true;
    }

    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    if (m_bPatched && m_pPatchAddr)
    {
        VirtualUnprotector unprotect(m_pPatchAddr, 1);
        *m_pPatchAddr = m_OriginalByte;
    }

    return true;
}

CGameEntitySystem* GameEntitySystem()
{
    return nullptr;
}

///////////////////////////////////////
const char* Plugin::GetLicense()
{
    return "GPLv3";
}

const char* Plugin::GetVersion()
{
    return VERSION_STRING;
}

const char* Plugin::GetDate()
{
    return BUILD_TIMESTAMP;
}

const char* Plugin::GetLogTag()
{
    return "HammerIdFix";
}

const char* Plugin::GetAuthor()
{
    return "Slynx";
}

const char* Plugin::GetDescription()
{
    return "Hammer id fix";
}

const char* Plugin::GetName()
{
    return "Hammer id fix";
}

const char* Plugin::GetURL()
{
    return "https://slynxdev.cz";
}
