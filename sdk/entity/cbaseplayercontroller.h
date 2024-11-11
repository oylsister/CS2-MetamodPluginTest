#pragma once

#include "ehandle.h"
#include "cbaseentity.h"

class CBasePlayerController : public CBaseEntiy
{
	int GetPlayerSlot() { return entindex() - 1; }
};