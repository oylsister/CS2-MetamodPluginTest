#pragma once

#include "ehandle.h"

class CBaseEntiy : public CEntityInstance
{
public:
	int entindex() { return m_pEntity->m_EHandle.GetEntryIndex(); }
};