#pragma once
#include "Event.h"
#include "netmessages.pb.h"

extern IGameEventManager2 *g_EventManager;

CUtlVector<CGameEventListener*> g_vecEventListeners;

void RegisterEventListeners()
{
	static bool bRegistered = false;

	if (bRegistered || !g_EventManager)
		return;

	FOR_EACH_VEC(g_vecEventListeners, i)
	{
		g_EventManager->AddListener(g_vecEventListeners[i], g_vecEventListeners[i]->GetEventName(), true);
	}

	bRegistered = true;
}

GAME_EVENT_F(player_spawn)
{
	auto pre = pEvent->GetPlayerController("userid");

	//ClientPrint
}