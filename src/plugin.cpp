// Author: Michal Přikryl (Slynx) <github.com/SlynxCZ>

#include "plugin.h"
#include "utils.hpp"

#include "memaddr.hpp"
#include "module.hpp"

#include "sdk/CServerSideClient.h"

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

ConVarRefAbstract* g_TvEnable = nullptr;

struct GameSessionConfiguration_t {
public:
    char pad[0x64]; // 0x0
    int maxPlayers; // 0x64
};

SH_DECL_HOOK2_void(CServerSideClient, Disconnect, SH_NOATTRIB, 0, ENetworkDisconnectionReason, const char*);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

    g_TvEnable = new ConVarRefAbstract("tv_enable");

    CModule libengine(g_pEngineServer);

    // VirtualTable Hooks
    {
        CMemory pCServerSideClientVTable = libengine.GetVirtualTableByName("CServerSideClient");

        m_iDisconnectHookID = SH_ADD_DVPHOOK(CServerSideClient, Disconnect, pCServerSideClientVTable.RCast<CServerSideClient*>(), SH_MEMBER(this, &Plugin::CServerSideClient_Disconnect), false);
        m_iStartupServerHookID = SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Plugin::INetworkServerService_StartupServer), false);
    }

    g_SMAPI->AddListener(this, this);

    ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    SH_REMOVE_HOOK_ID(m_iDisconnectHookID);
    SH_REMOVE_HOOK_ID(m_iStartupServerHookID);

    return true;
}

void Plugin::CServerSideClient_Disconnect(ENetworkDisconnectionReason reason, const char* pszInternalReason)
{
    CServerSideClient* pClient = META_IFACEPTR(CServerSideClient);
    if (pClient->m_bIsHLTV && g_TvEnable && g_TvEnable->GetBool())
    {
        META_LOG(this, "GOTV is enabled, blocking CServerSideClient disconnect\n");

        RETURN_META(MRES_SUPERCEDE);
    }

    RETURN_META(MRES_IGNORED);
}

void Plugin::INetworkServerService_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* session, const char*)
{
    if (g_TvEnable && g_TvEnable->GetBool())
    {
        META_LOG(this, "GOTV is enabled, expanding game session maxplayers (%i -> %i)\n", config.maxPlayers, config.maxPlayers + 1);
        const_cast<GameSessionConfiguration_t&>(config).maxPlayers++;
    }

    RETURN_META(MRES_IGNORED);
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
    return "DemoRecordFix";
}

const char* Plugin::GetAuthor()
{
    return "Slynx (˙·٠● S l y n x ●٠·˙)";
}

const char* Plugin::GetDescription()
{
    return "Demo record fix";
}

const char* Plugin::GetName()
{
    return "Demo record fix";
}

const char* Plugin::GetURL()
{
    return "https://slynxdev.cz";
}
