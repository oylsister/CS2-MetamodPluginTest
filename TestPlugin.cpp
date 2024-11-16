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
#include "netmessages.pb.h"

SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, ENetworkDisconnectionReason, const char *, uint64, const char *);
SH_DECL_HOOK4_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK8_void(IGameEventSystem, PostEventAbstract, SH_NOATTRIB, 0, CSplitScreenSlot, bool, int, const uint64*, INetworkMessageInternal*, const CNetMessage*, unsigned long, NetChannelBufType_t)

TestPlugin g_TestPlugin;
IGameEventSystem* g_gameEventSystem = nullptr;
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
IGameEventManager2 *gameevents = NULL;
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

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register( FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL );

	return true;
}

bool TestPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientDisconnect), true);
	SH_REMOVE_HOOK(IServerGameClients, ClientPutInServer, gameclients, SH_MEMBER(this, &TestPlugin::Hook_ClientPutInServer), true);

	return true;
}

void TestPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void SendVoiceData(CMsgVoiceAudio *audio)
{
	INetworkMessageInternal* pNetMsg = g_pNetworkMessages->FindNetworkMessageById(47);

	static void (IGameEventSystem:: * PostEventAbstract)(CSplitScreenSlot, bool, IRecipientFilter*, INetworkMessageInternal*, const CNetMessage*, unsigned long) = &IGameEventSystem::PostEventAbstract;

	auto data = pNetMsg->AllocateMessage()->ToPB<CSVCMsg_VoiceData>();

	CRecipientFilter* filter{};
	filter->AddAllPlayers();

	data->set_allocated_audio(audio);
	data->set_client(-1);
	data->set_proximity(true);
	data->set_xuid(0);
	data->set_audible_mask(0);
	data->set_tick(0);
	data->set_passthrough(0);

	SH_CALL(g_gameEventSystem, PostEventAbstract)(0, false, filter, pNetMsg, data, 0);
}

void TestPlugin::Hook_ClientPutInServer( CPlayerSlot slot, char const *pszName, int type, uint64 xuid )
{
	META_CONPRINTF( "Hook_ClientPutInServer(%d, \"%s\", %d, %d)\n", slot, pszName, type, xuid );
}

void TestPlugin::Hook_ClientDisconnect( CPlayerSlot slot, ENetworkDisconnectionReason reason, const char *pszName, uint64 xuid, const char *pszNetworkID )
{
	META_CONPRINTF( "Hook_ClientDisconnect(%d, %d, \"%s\", %d, \"%s\")\n", slot, reason, pszName, xuid, pszNetworkID );
}

void TestPlugin::OnLevelInit( char const *pMapName,
									 char const *pMapEntities,
									 char const *pOldLevel,
									 char const *pLandmarkName,
									 bool loadGame,
									 bool background )
{
	META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}

void TestPlugin::OnLevelShutdown()
{
	META_CONPRINTF("OnLevelShutdown()\n");
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
