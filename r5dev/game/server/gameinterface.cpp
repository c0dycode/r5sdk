//=============================================================================//
//
// Purpose: Implement things from GameInterface.cpp. Mostly the engine interfaces.
//
// $NoKeywords: $
//=============================================================================//

#include "core/stdafx.h"
#include "engine/server/sv_main.h"
#include "game/server/gameinterface.h"

//-----------------------------------------------------------------------------
// This is called when a new game is started. (restart, map)
//-----------------------------------------------------------------------------
void CServerGameDLL::GameInit(void)
{
	const static int index = 1;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// This is called when scripts are getting recompiled. (restart, map, changelevel)
//-----------------------------------------------------------------------------
void CServerGameDLL::PrecompileScriptsJob(void)
{
	const static int index = 2;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// Called when a level is shutdown (including changing levels)
//-----------------------------------------------------------------------------
void CServerGameDLL::LevelShutdown(void)
{
	const static int index = 8;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// This is called when a game ends (server disconnect, death, restart, load)
// NOT on level transitions within a game
//-----------------------------------------------------------------------------
void CServerGameDLL::GameShutdown(void)
{
	const static int index = 9;
	CallVFunc<void>(index, this);
}

//-----------------------------------------------------------------------------
// Purpose: Gets the simulation tick interfal
// Output : float
//-----------------------------------------------------------------------------
float CServerGameDLL::GetTickInterval(void)
{
	const static int index = 11;
	return CallVFunc<float>(index, this);
}

void __fastcall CServerGameDLL::OnReceivedSayTextMessage(void* thisptr, int senderId, const char* text, bool isTeamChat)
{
#if defined(GAMEDLL_S3)
	// set isTeamChat to false so that we can let the convar sv_forceChatToTeamOnly decide whether team chat should be enforced
	// this isn't a great way of doing it but it works so meh
	CServerGameDLL__OnReceivedSayTextMessage(thisptr, senderId, text, false);
#endif
}

void CServerGameDLL_Attach()
{
#if defined(GAMEDLL_S3)
	DetourAttach((LPVOID*)&CServerGameDLL__OnReceivedSayTextMessage, &CServerGameDLL::OnReceivedSayTextMessage);
#endif
}

void CServerGameDLL_Detach()
{
#if defined(GAMEDLL_S3)
	DetourDetach((LPVOID*)&CServerGameDLL__OnReceivedSayTextMessage, &CServerGameDLL::OnReceivedSayTextMessage);
#endif
}

// Pointer to CServerGameDLL virtual function table.
CServerGameDLL* g_pServerGameDLL = nullptr;
CServerGameClients* g_pServerGameClients = nullptr;
