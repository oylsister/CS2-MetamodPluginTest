#pragma once
//#include "common.h"
#include "utlstring.h"
#include "utlvector.h"
#include "igameevents.h"

typedef void (*FnEventListenerCallback)(IGameEvent* event);

class CGameEventListener;

extern CUtlVector<CGameEventListener*> g_vecEventListeners;

class CGameEventListener : public IGameEventListener2
{
public:
	CGameEventListener(FnEventListenerCallback callback, const char* pszEventName) : m_Callback(callback), m_pszEventName(pszEventName)
	{
		g_vecEventListeners.AddToTail(this);
	}

	~CGameEventListener() override
	{
	}

	// FireEvent is called by EventManager if event just occured
	// KeyValue memory will be freed by manager if not needed anymore
	void FireGameEvent(IGameEvent* event) override
	{
		m_Callback(event);
	}

	const char* GetEventName() { return m_pszEventName; }

private:
	FnEventListenerCallback m_Callback;
	const char* m_pszEventName;
};

void RegisterEventListeners();

#define GAME_EVENT_F(_event)												\
	void _event##_callback(IGameEvent*);									\
	CGameEventListener _event##_listener(_event##_callback, #_event);		\
	void _event##_callback(IGameEvent *pEvent)
