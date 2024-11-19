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
#include <iaudioplayer.h>

SH_DECL_HOOK8_void(IGameEventSystem, PostEventAbstract, SH_NOATTRIB, 0, CSplitScreenSlot, bool, int, const uint64*, INetworkMessageInternal*, const CNetMessage*, unsigned long, NetChannelBufType_t)

TestPlugin g_TestPlugin;
IAudioPlayer* pAudioPlayer = nullptr;
IGameEventSystem* g_gameEventSystem = nullptr;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars* GetGameGlobals()
{
	INetworkGameServer* server = g_pNetworkServerService->GetIGameServer();

	if (!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}	


PLUGIN_EXPOSE(TestPlugin, g_TestPlugin);
bool TestPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetEngineFactory, g_gameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);
	SH_ADD_HOOK(IGameEventSystem, PostEventAbstract, g_gameEventSystem, SH_MEMBER(this, &TestPlugin::Hook_PostEvent), false);

	g_SMAPI->AddListener( this, this );
	return true;

}

bool TestPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IGameEventSystem, PostEventAbstract, g_gameEventSystem, SH_MEMBER(this, &TestPlugin::Hook_PostEvent), false);
	return true;
}

void TestPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */

	pAudioPlayer = (IAudioPlayer*)g_SMAPI->MetaFactory(AUDIOPLAYER_INTERFACE, nullptr, nullptr);
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

DLL_EXPORT void PlaySoundFromSharp(const char* filename)
{
	const char* pszJsonPath = "addons/TestPlugin/";
	char szPath[MAX_PATH];
	V_snprintf(szPath, sizeof(szPath), "%s%s%s%s", Plat_GetGameDirectory(), "/csgo/", pszJsonPath ,filename);

	std::string buffer(szPath);

	//Message(szPath);
	//Message(buffer);

	pAudioPlayer->PlayAudio(buffer, 1.0);
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
		if (param2 == "!tutu")
		{
			PlaySoundFromSharp("tuturu.wav");
		}
	}
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
