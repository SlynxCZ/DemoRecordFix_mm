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

class GameSessionConfiguration_t
{
};

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);

    m_iStartupServerHookID = SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Plugin::INetworkServerService_StartupServer), true);

    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    SH_REMOVE_HOOK_ID(m_iStartupServerHookID);

    return true;
}

void Plugin::INetworkServerService_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession*,
    const char*)
{
    CModule libserver(g_pSource2Server);

    // https://github.com/Source2ZE/CS2Fixes/commit/61937f78dd649ed391f6988b0c58ae4a75fd4bc6
    {
        CModule libserver(g_pSource2Server);


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
