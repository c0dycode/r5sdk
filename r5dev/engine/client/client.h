#pragma once
#include "vpc/keyvalues.h"
#include "common/protocol.h"
#include "engine/net_chan.h"

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
enum Reputation_t
{
	REP_NONE = 0,
	REP_REMOVE_ONLY,
	REP_MARK_BAD
};

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CServer;
class CClient;

///////////////////////////////////////////////////////////////////////////////
extern CClient* g_pClient;

class CClient : IClientMessageHandler, INetChannelHandler
{
public:
	CClient* GetClient(int nIndex) const;
	uint16_t GetHandle(void) const;
	uint32_t GetUserID(void) const;
	uint64_t GetNucleusID(void) const;
	SIGNONSTATE GetSignonState(void) const;
	PERSISTENCE GetPersistenceState(void) const;
	CNetChan* GetNetChan(void) const;
	const char* GetServerName(void) const;
	const char* GetClientName(void) const;
	void SetHandle(uint16_t nHandle);
	void SetUserID(uint32_t nUserID);
	void SetNucleusID(uint64_t nNucleusID);
	void SetSignonState(SIGNONSTATE nSignonState);
	void SetPersistenceState(PERSISTENCE nPersistenceState);
	void SetNetChan(CNetChan* pNetChan);
	bool IsConnected(void) const;
	bool IsSpawned(void) const;
	bool IsActive(void) const;
	bool IsPersistenceAvailable(void) const;
	bool IsPersistenceReady(void) const;
	bool IsFakeClient(void) const;
	bool IsHumanPlayer(void) const;
	bool Connect(const char* szName, void* pNetChannel, bool bFakePlayer, void* a5, char* szMessage, int nMessageSize);
	void Disconnect(const Reputation_t nRepLvl, const char* szReason, ...);
	static bool VConnect(CClient* pClient, const char* szName, void* pNetChannel, bool bFakePlayer, void* a5, char* szMessage, int nMessageSize);
	void Clear(void);
	static void VClear(CClient* pClient);
	static bool VProcessStringCmd(CClient* pClient, NET_StringCmd* pMsg);

private:
	uint32_t m_nUserID;              //0x0010
	uint16_t m_nHandle;              //0x0014
	char m_szServerName[64];         //0x0016
	int64_t m_nReputation;           //0x0058
	char pad_0014[182];              //0x0060
	char m_szClientName[64];         //0x0116
	char pad_0015[522];              //0x0156
	KeyValues* m_ConVars;            //0x0360
	char pad_0368[8];                //0x0368
	CServer* m_pServer;              //0x0370
	char pad_0378[40];               //0x0378
	CNetChan* m_NetChannel;          //0x03A0
	char pad_03A8[8];                //0x03A8
	SIGNONSTATE m_nSignonState;      //0x03B0
	int32_t m_nDeltaTick;            //0x03B4
	uint64_t m_nNucleusID;           //0x03B8
	int32_t m_nStringTableAckTick;   //0x03BC
	int32_t m_nSignonTick;           //0x03C0
	char pad_03C0[464];              //0x03C4
#if defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
	char pad_0598[8];                //0x0598
#endif
	bool m_bFakePlayer;              //0x05A0
	bool m_bReceivedPacket;          //0x05A1
	bool m_bLowViolence;             //0x05A2
	bool m_bFullyAuthenticated;      //0x05A3
	char pad_05A4[24];               //0x05A4
	PERSISTENCE m_nPersistenceState; //0x05BC
	char pad_05C0[302676];           //0x05C0
	int32_t m_LastMovementTick;      //0x4A414
#if defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
	char pad_4A418[120];             //0x4A418
#endif
	char pad_4A440[48];              //0x4A440
};
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
static_assert(sizeof(CClient) == 0x4A440);
#else
static_assert(sizeof(CClient) == 0x4A4C0);
#endif


/* ==== CBASECLIENT ===================================================================================================================================================== */
inline CMemory p_CClient_Connect;
inline auto v_CClient_Connect = p_CClient_Connect.RCast<bool (*)(CClient* pClient, const char* szName, void* pNetChannel, bool bFakePlayer, void* a5, char* szMessage, int nMessageSize)>();

inline CMemory p_CClient_Disconnect;
inline auto v_CClient_Disconnect = p_CClient_Disconnect.RCast<bool (*)(CClient* pClient, const Reputation_t nRepLvl, const char* szReason, ...)>();

inline CMemory p_CClient_Clear;
inline auto v_CClient_Clear = p_CClient_Clear.RCast<void (*)(CClient* pClient)>();

inline CMemory p_CClient_ProcessStringCmd;
inline auto v_CClient_ProcessStringCmd = p_CClient_ProcessStringCmd.RCast<bool (*)(CClient* pClient, NET_StringCmd* pMsg)>();

///////////////////////////////////////////////////////////////////////////////
void CBaseClient_Attach();
void CBaseClient_Detach();

///////////////////////////////////////////////////////////////////////////////
class VClient : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: CClient::Connect                     : {:#18x} |\n", p_CClient_Connect.GetPtr());
		spdlog::debug("| FUN: CClient::Disconnect                  : {:#18x} |\n", p_CClient_Disconnect.GetPtr());
		spdlog::debug("| FUN: CClient::Clear                       : {:#18x} |\n", p_CClient_Clear.GetPtr());
		spdlog::debug("| FUN: CClient::ProcessStringCmd            : {:#18x} |\n", p_CClient_ProcessStringCmd.GetPtr());
		spdlog::debug("| VAR: g_pClient[128]                       : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pClient));
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
		p_CClient_Connect    = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x56\x48\x83\xEC\x20\x41\x0F\xB6\xE9"), "xxxx?xxxx?xxxxxxxxxxxx");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
		p_CClient_Disconnect = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x8B\xC4\x4C\x89\x40\x18\x4C\x89\x48\x20\x53\x56\x57\x48\x81\xEC\x00\x00\x00\x00\x83\xB9\x00\x00\x00\x00\x00\x49\x8B\xF8\x0F\xB6\xF2"), "xxxxxxxxxxxxxxxxx????xx?????xxxxxx");
#else // !GAMEDLL_S0 || !GAMEDLL_S1 || !GAMEDLL_S2
		p_CClient_Disconnect = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x8B\xC4\x4C\x89\x40\x18\x4C\x89\x48\x20\x53\x56\x57\x48\x81\xEC\x00\x00\x00\x00\x83\xB9\x00\x00\x00\x00\x00\x49\x8B\xF8\x8B\xF2"), "xxxxxxxxxxxxxxxxx????xx?????xxxxx");
#endif
		p_CClient_Clear      = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x41\x56\x41\x57\x48\x83\xEC\x20\x48\x8B\xD9\x48\x89\x74"), "xxxxxxxxxxxxxxxx");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_CClient_ProcessStringCmd = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x83\xEC\x28\x4C\x8B\x42\x20"), "xxxxxxxx");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		p_CClient_ProcessStringCmd = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x6C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x7A\x20"), "xxxx?xxxx????xxxx");
#endif // !GAMEDLL_S0 || !GAMEDLL_S1

		v_CClient_Connect    = p_CClient_Connect.RCast<bool (*)(CClient*, const char*, void*, bool, void*, char*, int)>(); /*48 89 5C 24 ?? 48 89 6C 24 ?? 56 57 41 56 48 83 EC 20 41 0F B6 E9*/
		v_CClient_Disconnect = p_CClient_Disconnect.RCast<bool (*)(CClient*, const Reputation_t, const char*, ...)>();     /*48 8B C4 4C 89 40 18 4C 89 48 20 53 56 57 48 81 EC ?? ?? ?? ?? 83 B9 ?? ?? ?? ?? ?? 49 8B F8 8B F2*/
		v_CClient_Clear      = p_CClient_Clear.RCast<void (*)(CClient*)>();                                                /*40 53 41 56 41 57 48 83 EC 20 48 8B D9 48 89 74*/
		v_CClient_ProcessStringCmd = p_CClient_ProcessStringCmd.RCast<bool (*)(CClient*, NET_StringCmd*)>();               /*48 89 6C 24 ?? 57 48 81 EC ?? ?? ?? ?? 48 8B 7A 20*/
	}
	virtual void GetVar(void) const
	{
		g_pClient = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x3B\x15\x00\x00\x00\x00\x7D\x33"), "xx????xx")
			.FindPatternSelf("48 8D 0D", CMemory::Direction::DOWN, 150).ResolveRelativeAddressSelf(0x3, 0x7).RCast<CClient*>();
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VClient);
