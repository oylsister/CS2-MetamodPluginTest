#pragma once
#include "irecipientfilter.h"
//#include "playermanager.h"

// Simple filter for when only 1 recipient is needed
class CSingleRecipientFilter : public IRecipientFilter
{
public:
	CSingleRecipientFilter(CPlayerSlot iRecipient, NetChannelBufType_t nBufType = BUF_RELIABLE, bool bInitMessage = false) :
		m_iRecipient(iRecipient), m_nBufType(nBufType), m_bInitMessage(bInitMessage) {}

	~CSingleRecipientFilter() override {}

	NetChannelBufType_t GetNetworkBufType(void) const override { return m_nBufType; }
	bool IsInitMessage(void) const override { return m_bInitMessage; }
	int GetRecipientCount(void) const override { return 1; }
	CPlayerSlot GetRecipientIndex(int slot) const override { return m_iRecipient; }

private:
	CPlayerSlot m_iRecipient;
	NetChannelBufType_t m_nBufType;
	bool m_bInitMessage;
};


class CRecipientFilter : public IRecipientFilter
{
public:
	CRecipientFilter()
	{
		m_nBufType = BUF_RELIABLE;
		m_bInitMessage = false;
	}

	CRecipientFilter(IRecipientFilter* source, int iExcept = -1)
	{
		m_nBufType = source->GetNetworkBufType();
		m_bInitMessage = source->IsInitMessage();
		m_Recipients.RemoveAll();

		for (int i = 0; i < source->GetRecipientCount(); i++)
		{
			if (source->GetRecipientIndex(i).Get() != iExcept)
				m_Recipients.AddToTail(source->GetRecipientIndex(i));
		}
	}

	~CRecipientFilter() override {}

	NetChannelBufType_t GetNetworkBufType(void) const override { return m_nBufType; }
	bool IsInitMessage(void) const override { return m_bInitMessage; }
	int GetRecipientCount(void) const override { return m_Recipients.Count(); }

	CPlayerSlot GetRecipientIndex(int slot) const override
	{
		if (slot < 0 || slot >= GetRecipientCount())
			return CPlayerSlot(-1);

		return m_Recipients[slot];
	}

	void AddAllPlayers(void)
	{
		m_Recipients.RemoveAll();

		for (int i = 0; i < 64; i++)
		{
			//if (!g_playerManager->GetPlayer(i))
				//continue;

			AddRecipient(i);
		}
	}

	void AddRecipient(CPlayerSlot slot)
	{
		// Don't add if it already exists
		if (m_Recipients.Find(slot) != m_Recipients.InvalidIndex())
			return;

		m_Recipients.AddToTail(slot);
	}

private:
	NetChannelBufType_t m_nBufType;
	bool m_bInitMessage;
	CUtlVectorFixed<CPlayerSlot, 64> m_Recipients;
};