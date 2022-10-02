//=============================================================================//
//
// Purpose: Interface the engine exposes to the game DLL
//
//=============================================================================//

#include "core/stdafx.h"
#include "tier1/cvar.h"
#include "common/protocol.h"
#include "engine/client/client.h"
#include "server/vengineserver_impl.h"

//-----------------------------------------------------------------------------
// Purpose: sets the persistence var in the CClient instance to 'ready'
//-----------------------------------------------------------------------------
bool HIVEngineServer__PersistenceAvailable(void* entidx, int clienthandle)
{
	CClient* pClient = g_pClient->GetClient(clienthandle);        // Get client instance.
	pClient->SetPersistenceState(PERSISTENCE::PERSISTENCE_READY); // Set the client instance to 'ready'.

	if (!g_ServerPlayer[clienthandle].m_bPersistenceEnabled && sv_showconnecting->GetBool())
	{
		CNetChan* pNetChan = pClient->GetNetChan();

		string svClientName = pNetChan->GetName();
		string svIpAddress = pNetChan->GetAddress();
		uint64_t nNucleusID = pClient->GetNucleusID();

		DevMsg(eDLL_T::SERVER, "______________________________________________________________\n");
		DevMsg(eDLL_T::SERVER, "+- Enabled persistence for NetChannel:\n");
		DevMsg(eDLL_T::SERVER, " |- IDX : | '#%d'\n", clienthandle);
		DevMsg(eDLL_T::SERVER, " |- UID : | '%s'\n", svClientName.c_str());
		DevMsg(eDLL_T::SERVER, " |- PID : | '%llu'\n", nNucleusID);
		DevMsg(eDLL_T::SERVER, " |- ADR : | '%s'\n", svIpAddress.c_str());
		DevMsg(eDLL_T::SERVER, " -------------------------------------------------------------\n");

		g_ServerPlayer[clienthandle].m_bPersistenceEnabled = true;
	}
	///////////////////////////////////////////////////////////////////////////
	return IVEngineServer__PersistenceAvailable(entidx, clienthandle);
}

void IVEngineServer_Attach()
{
	DetourAttach((LPVOID*)&IVEngineServer__PersistenceAvailable, &HIVEngineServer__PersistenceAvailable);
}

void IVEngineServer_Detach()
{
	DetourDetach((LPVOID*)&IVEngineServer__PersistenceAvailable, &HIVEngineServer__PersistenceAvailable);
}

///////////////////////////////////////////////////////////////////////////////
ServerPlayer_t g_ServerPlayer[MAX_PLAYERS];
