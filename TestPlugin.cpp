/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#include <stdio.h>
#include "TestPlugin.h"
#include "iserver.h"
#include "usermessages.pb.h"
#include "engine/igameeventsystem.h"
#include <fstream>

SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, ENetworkDisconnectionReason, const char *, uint64, const char *);
SH_DECL_HOOK4_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK8_void(IGameEventSystem, PostEventAbstract, SH_NOATTRIB, 0, CSplitScreenSlot, bool, int, const uint64*, INetworkMessageInternal*, const CNetMessage*, unsigned long, NetChannelBufType_t)

TestPlugin g_TestPlugin;
IGameEventSystem* g_gameEventSystem = nullptr;
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
ICvar *icvar = NULL;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

#if 0
// Currently unavailable, requires hl2sdk work!
ConVar sample_cvar("sample_cvar", "42", 0);
#endif

PLUGIN_EXPOSE(TestPlugin, g_TestPlugin);
bool TestPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_gameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	// Required to get the IMetamodListener events
	g_SMAPI->AddListener( this, this );

	META_CONPRINTF( "Starting plugin.\n" );

	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientDisconnect), true);
	SH_ADD_HOOK(IServerGameClients, ClientPutInServer, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientPutInServer), true);
	SH_ADD_HOOK(IGameEventSystem, PostEventAbstract, g_gameEventSystem, SH_MEMBER(this, &TestPlugin::Hook_PostEvent), false);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register( FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL );

	return true;
}

bool TestPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientDisconnect), true);
	SH_REMOVE_HOOK(IServerGameClients, ClientPutInServer, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientPutInServer), true);
	SH_REMOVE_HOOK(IGameEventSystem, PostEventAbstract, g_gameEventSystem, SH_MEMBER(this, &TestPlugin::Hook_PostEvent), false);

	return true;
}

void TestPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void Message(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 255, 255), "[Sympho] %s", buf);

	va_end(args);
}

void Panic(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	Warning("[Sympho] %s", buf);

	va_end(args);
}

void TestPlugin::Hook_ClientPutInServer( CPlayerSlot slot, char const *pszName, int type, uint64 xuid )
{
	//META_CONPRINTF( "Hook_ClientPutInServer(%d, \"%s\", %d, %d)\n", slot, pszName, type, xuid );
}

void TestPlugin::Hook_ClientDisconnect( CPlayerSlot slot, ENetworkDisconnectionReason reason, const char *pszName, uint64 xuid, const char *pszNetworkID )
{
	//META_CONPRINTF( "Hook_ClientDisconnect(%d, %d, \"%s\", %d, \"%s\")\n", slot, reason, pszName, xuid, pszNetworkID );
}

void TestPlugin::Hook_PostEvent(CSplitScreenSlot nSlot, bool bLocalOnly, int nClientCount, const uint64* clients, INetworkMessageInternal* pEvent, const CNetMessage* pData, unsigned long nSize, NetChannelBufType_t bufType)
{
	static void (IGameEventSystem:: * PostEventAbstract)(CSplitScreenSlot, bool, int, const uint64*,
		INetworkMessageInternal*, const CNetMessage*, unsigned long, NetChannelBufType_t) = &IGameEventSystem::PostEventAbstract;

	NetMessageInfo_t* info = pEvent->GetNetMessageInfo();

	if (info->m_MessageId == UM_SayText2)
	{
		auto msg = const_cast<CNetMessage*>(pData)->ToPB<CUserMessageSayText2>();

		auto param2 = msg->param2();

		// if param is not ! then ignore it.
		if (param2[0] != '!')
		{
			return;
		}
	}
}

void LoadAudioConfig()
{
	const char* configPath = "addons/sympho/configs.jsonc";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s", Plat_GetGameDirectory(), "/csgo/", configPath);

	// open file
	std::ifstream jsoncFile(szPath);

	if (!jsoncFile.is_open())
	{
		Panic("Failed to open %s. Sound file is not loaded\n", configPath);
		return;
	}
}

void TestPlugin::OnLevelInit( char const *pMapName,
									 char const *pMapEntities,
									 char const *pOldLevel,
									 char const *pLandmarkName,
									 bool loadGame,
									 bool background )
{
	//META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}



void TestPlugin::OnLevelShutdown()
{
	//META_CONPRINTF("OnLevelShutdown()\n");
}

bool TestPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool TestPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *TestPlugin::GetLicense()
{
	return "Public Domain";
}

const char *TestPlugin::GetVersion()
{
	return "1.0";
}

const char *TestPlugin::GetDate()
{
	return __DATE__;
}

const char *TestPlugin::GetLogTag()
{
	return "TEST";
}

const char *TestPlugin::GetAuthor()
{
	return "Oylsister";
}

const char *TestPlugin::GetDescription()
{
	return "Sample Test plugin";
}

const char *TestPlugin::GetName()
{
	return "Test Plugin";
}

const char *TestPlugin::GetURL()
{
	return "https://github.com/Oylsister";
}
