// Author: Michal Přikryl (Slynx) <github.com/SlynxCZ>

#include "plugin.h"
#include "utils.hpp"

#include "mempatch.h"
#include "macros.h"
#include "memaddr.hpp"
#include "module.hpp"
#include "virtual.hpp"
#include "vthook.hpp"

#include "eiface.h"
#include "iserver.h"
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

CMemPatch m_HammerPatch{"SetSchemaHammerUniqueId"};

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);

    CModule libserver(g_pSource2Server);

    // https://github.com/Source2ZE/CS2Fixes/commit/61937f78dd649ed391f6988b0c58ae4a75fd4bc6
    {
        static uint8_t patch[] = { 0xEB };

        if (!m_HammerPatch.PerformPatch(libserver.FindPattern(ParseStringPattern(WIN_LINUX("75 ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 84 C0 74 ? 48 8D 05", "75 ? 48 8B 03 48 8D 15 ? ? ? ? 48 8B 80 ? ? ? ? 48 39 D0 75 ? 48 83 C4"))), patch, sizeof(patch), 0))
        {
            META_LOG(this, "Failed to apply SetSchemaHammerUniqueId patch\n");
        }
        else
        {
            META_LOG(this, "SetSchemaHammerUniqueId patched (jnz -> jmp)\n");
        }
    }

    return true;
}

///////////////////////////////////////

CGameEntitySystem* GameEntitySystem()
{
    // CGameResourceService::SetEntityResourceManifest
    // str server_entities
    return *CMemory(g_pGameResourceServiceServer).Offset(WIN_LINUX(0x58, 0x50)).RCast<CGameEntitySystem**>();
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
    return "Slynx (˙·٠● S l y n x ●٠·˙)";
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
