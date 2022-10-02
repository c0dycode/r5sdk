//=============================================================================//
//
// Purpose: Console Commands
//
//=============================================================================//

#include "core/stdafx.h"
#include "tier0/tslist.h"
#include "tier0/memstd.h"
#include "tier0/commandline.h"
#include "tier1/cmd.h"
#include "tier1/cvar.h"
#include "tier1/characterset.h"
#include "vstdlib/callback.h"

//-----------------------------------------------------------------------------
// Global methods
//-----------------------------------------------------------------------------
static characterset_t s_BreakSet;
static bool s_bBuiltBreakSet = false;


//-----------------------------------------------------------------------------
// Tokenizer class
//-----------------------------------------------------------------------------
CCommand::CCommand()
{
	if (!s_bBuiltBreakSet)
	{
		s_bBuiltBreakSet = true;
		CharacterSetBuild(&s_BreakSet, "{}()':");
	}

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: constructor
// Input  : nArgC - 
//			**ppArgV - 
//			source - 
//-----------------------------------------------------------------------------
CCommand::CCommand(int nArgC, const char** ppArgV, cmd_source_t source)
{
	Assert(nArgC > 0);

	if (!s_bBuiltBreakSet)
	{
		s_bBuiltBreakSet = true;
		CharacterSetBuild(&s_BreakSet, "{}()':");
	}

	Reset();

	char* pBuf = m_pArgvBuffer;
	char* pSBuf = m_pArgSBuffer;
	m_nArgc = nArgC;
	for (int i = 0; i < nArgC; ++i)
	{
		m_ppArgv[i] = pBuf;
		int nLen = strlen(ppArgV[i]);
		memcpy(pBuf, ppArgV[i], nLen + 1);
		if (i == 0)
		{
			m_nArgv0Size = nLen;
		}
		pBuf += nLen + 1;

		bool bContainsSpace = strchr(ppArgV[i], ' ') != NULL;
		if (bContainsSpace)
		{
			*pSBuf++ = '\"';
		}
		memcpy(pSBuf, ppArgV[i], nLen);
		pSBuf += nLen;
		if (bContainsSpace)
		{
			*pSBuf++ = '\"';
		}

		if (i != nArgC - 1)
		{
			*pSBuf++ = ' ';
		}
	}

	m_nQueuedVal = source;
}

//-----------------------------------------------------------------------------
// Purpose: tokenizer
// Input  : *pCommand - 
//			source - 
//			*pBreakSet - 
// Output : true on success, false on failure
//-----------------------------------------------------------------------------
bool CCommand::Tokenize(const char* pCommand, cmd_source_t source, characterset_t* pBreakSet)
{
	/* !TODO (CUtlBuffer).
	Reset();
	m_nQueuedVal = source;

	if (!pCommand)
		return false;

	// Use default break set
	if (!pBreakSet)
	{
		pBreakSet = &s_BreakSet;
	}

	// Copy the current command into a temp buffer
	// NOTE: This is here to avoid the pointers returned by DequeueNextCommand
	// to become invalid by calling AddText. Is there a way we can avoid the memcpy?
	int nLen = Q_strlen(pCommand);
	if (nLen >= COMMAND_MAX_LENGTH - 1)
	{
		Warning(eDLL_T::ENGINE, "%s: Encountered command which overflows the tokenizer buffer.. Skipping!\n", __FUNCTION__);
		return false;
	}

	memcpy(m_pArgSBuffer, pCommand, nLen + 1);

	// Parse the current command into the current command buffer
	CUtlBuffer bufParse(m_pArgSBuffer, nLen, CUtlBuffer::TEXT_BUFFER | CUtlBuffer::READ_ONLY);
	int nArgvBufferSize = 0;
	while (bufParse.IsValid() && (m_nArgc < COMMAND_MAX_ARGC))
	{
		char* pArgvBuf = &m_pArgvBuffer[nArgvBufferSize];
		int nMaxLen = COMMAND_MAX_LENGTH - nArgvBufferSize;
		int nStartGet = bufParse.TellGet();
		int	nSize = bufParse.ParseToken(pBreakSet, pArgvBuf, nMaxLen);
		if (nSize < 0)
			break;

		// Check for overflow condition
		if (nMaxLen == nSize)
		{
			Reset();
			return false;
		}

		if (m_nArgc == 1)
		{
			// Deal with the case where the arguments were quoted
			m_nArgv0Size = bufParse.TellGet();
			bool bFoundEndQuote = m_pArgSBuffer[m_nArgv0Size - 1] == '\"';
			if (bFoundEndQuote)
			{
				--m_nArgv0Size;
			}
			m_nArgv0Size -= nSize;
			Assert(m_nArgv0Size != 0);

			// The StartGet check is to handle this case: "foo"bar
			// which will parse into 2 different args. ArgS should point to bar.
			bool bFoundStartQuote = (m_nArgv0Size > nStartGet) && (m_pArgSBuffer[m_nArgv0Size - 1] == '\"');
			Assert(bFoundEndQuote == bFoundStartQuote);
			if (bFoundStartQuote)
			{
				--m_nArgv0Size;
			}
		}

		m_ppArgv[m_nArgc++] = pArgvBuf;
		if (m_nArgc >= COMMAND_MAX_ARGC)
		{
			Warning(eDLL_T::ENGINE, "%s: Encountered command which overflows the argument buffer.. Clamped!\n", __FUNCTION__);
		}

		nArgvBufferSize += nSize + 1;
		Assert(nArgvBufferSize <= COMMAND_MAX_LENGTH);
	}*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns argument count
//-----------------------------------------------------------------------------
int64_t CCommand::ArgC(void) const
{
	return m_nArgc;
}

//-----------------------------------------------------------------------------
// Purpose: returns argument vector
//-----------------------------------------------------------------------------
const char** CCommand::ArgV(void) const
{
	return m_nArgc ? (const char**)m_ppArgv : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: returns all args that occur after the 0th arg, in string form
//-----------------------------------------------------------------------------
const char* CCommand::ArgS(void) const
{
	return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}

//-----------------------------------------------------------------------------
// Purpose: returns the entire command in string form, including the 0th arg
//-----------------------------------------------------------------------------
const char* CCommand::GetCommandString(void) const
{
	return m_nArgc ? m_pArgSBuffer : "";
}

//-----------------------------------------------------------------------------
// Purpose: returns argument from index as string
// Input  : nIndex - 
//-----------------------------------------------------------------------------
const char* CCommand::Arg(int nIndex) const
{
	// FIXME: Many command handlers appear to not be particularly careful
	// about checking for valid argc range. For now, we're going to
	// do the extra check and return an empty string if it's out of range
	if (nIndex < 0 || nIndex >= m_nArgc)
	{
		return "";
	}
	return m_ppArgv[nIndex];
}

//-----------------------------------------------------------------------------
// Purpose: gets at arguments
// Input  : nInput - 
//-----------------------------------------------------------------------------
const char* CCommand::operator[](int nIndex) const
{
	return Arg(nIndex);
}

//-----------------------------------------------------------------------------
// Purpose: returns max command length
//-----------------------------------------------------------------------------
int CCommand::MaxCommandLength(void) const
{
	return COMMAND_MAX_LENGTH - 1;
}


//-----------------------------------------------------------------------------
// Purpose: return boolean depending on if the string only has digits in it
// Input  : svString - 
//-----------------------------------------------------------------------------
bool CCommand::HasOnlyDigits(int nIndex) const
{
	const string svString = Arg(nIndex);
	for (const char& character : svString)
	{
		if (std::isdigit(character) == 0)
		{
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: reset
//-----------------------------------------------------------------------------
void CCommand::Reset()
{
	m_nArgc = 0;
	m_nArgv0Size = 0;
	m_pArgSBuffer[0] = 0;
	m_nQueuedVal = cmd_source_t::kCommandSrcInvalid;
}

//-----------------------------------------------------------------------------
// Purpose: create
//-----------------------------------------------------------------------------
ConCommand* ConCommand::Create(const char* pszName, const char* pszHelpString, int nFlags, FnCommandCallback_t pCallback, FnCommandCompletionCallback pCompletionFunc)
{
	ConCommand* pCommand = MemAllocSingleton()->Alloc<ConCommand>(sizeof(ConCommand));
	memset(pCommand, '\0', sizeof(ConCommand));

	pCommand->m_pConCommandBaseVFTable = g_pConCommandVFTable.RCast<IConCommandBase*>();
	pCommand->m_pszName = pszName;
	pCommand->m_pszHelpString = pszHelpString;
	pCommand->m_nFlags = nFlags;
	pCommand->m_nNullCallBack = NullSub;
	pCommand->m_fnCommandCallback = pCallback;
	pCommand->m_bHasCompletionCallback = pCompletionFunc != nullptr ? true : false;
	pCommand->m_bUsingNewCommandCallback = true;
	pCommand->m_bUsingCommandCallbackInterface = false;
	pCommand->m_fnCompletionCallback = pCompletionFunc ? pCompletionFunc : CallbackStub;

	g_pCVar->RegisterConCommand(pCommand);

	return pCommand;
}

//-----------------------------------------------------------------------------
// Purpose: construct/allocate
//-----------------------------------------------------------------------------
ConCommand::ConCommand()
	: m_nNullCallBack(nullptr)
	, m_pSubCallback(nullptr)
	, m_fnCommandCallbackV1(nullptr)
	, m_fnCompletionCallback(nullptr)
	, m_bHasCompletionCallback(false)
	, m_bUsingNewCommandCallback(false)
	, m_bUsingCommandCallbackInterface(false)
{
}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
ConCommand::~ConCommand()
{
}

//-----------------------------------------------------------------------------
// Purpose: ConCommand registration
//-----------------------------------------------------------------------------
void ConCommand::Init(void)
{
	//-------------------------------------------------------------------------
	// ENGINE DLL                                                             |
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1)
	ConCommand::Create("bhit", "Bullet-hit trajectory debug.", FCVAR_DEVELOPMENTONLY | FCVAR_GAMEDLL, BHit_f, nullptr);
#endif // !GAMEDLL_S0 && !GAMEDLL_S1
#ifndef DEDICATED
	ConCommand::Create("line", "Draw a debug line.", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT, Line_f, nullptr);
	ConCommand::Create("sphere", "Draw a debug sphere.", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT, Sphere_f, nullptr);
	ConCommand::Create("capsule", "Draw a debug capsule.", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT, Capsule_f, nullptr);
#endif //!DEDICATED
#ifndef CLIENT_DLL
	ConCommand::Create("reload_playlists", "Reloads the playlists file.", FCVAR_RELEASE, Host_ReloadPlaylists_f, nullptr);
#endif // !CLIENT_DLL
	//-------------------------------------------------------------------------
	// SERVER DLL                                                             |
#ifndef CLIENT_DLL
	ConCommand::Create("script", "Run input code as SERVER script on the VM.", FCVAR_GAMEDLL | FCVAR_CHEAT, SQVM_ServerScript_f, nullptr);
	ConCommand::Create("sv_kick", "Kick a client from the server by user name. | Usage: sv_kick \"<UserID>\".", FCVAR_RELEASE, Host_Kick_f, nullptr);
	ConCommand::Create("sv_kickid", "Kick a client from the server by handle or nucleus id | Usage: sv_kickid \"<HandleID>\"/\"<NucleusID>\".", FCVAR_RELEASE, Host_KickID_f, nullptr);
	ConCommand::Create("sv_ban", "Bans a client from the server by user name. | Usage: sv_ban <UserID>.", FCVAR_RELEASE, Host_Ban_f, nullptr);
	ConCommand::Create("sv_banid", "Bans a client from the server by handle, nucleus id or ip address | Usage: sv_banid \"<HandleID>\"/\"<NucleusID>/<IPAddress>\".", FCVAR_RELEASE, Host_BanID_f, nullptr);
	ConCommand::Create("sv_unban", "Unbans a client from the server by nucleus id or ip address | Usage: sv_unban \"<NucleusID>\"/\"<IPAddress>\".", FCVAR_RELEASE, Host_Unban_f, nullptr);
	ConCommand::Create("sv_reloadbanlist", "Reloads the banned list.", FCVAR_RELEASE, Host_ReloadBanList_f, nullptr);
#endif // !CLIENT_DLL
#ifndef DEDICATED
	//-------------------------------------------------------------------------
	// CLIENT DLL                                                             |
	ConCommand::Create("script_client", "Run input code as CLIENT script on the VM.", FCVAR_CLIENTDLL | FCVAR_CHEAT, SQVM_ClientScript_f, nullptr);
	ConCommand::Create("cl_showconsole", "Opens the game console.", FCVAR_CLIENTDLL | FCVAR_RELEASE, GameConsole_Invoke_f, nullptr);
	ConCommand::Create("cl_showbrowser", "Opens the server browser.", FCVAR_CLIENTDLL | FCVAR_RELEASE, ServerBrowser_Invoke_f, nullptr);
	ConCommand::Create("rcon", "Forward RCON query to remote server. | Usage: rcon \"<query>\".", FCVAR_CLIENTDLL | FCVAR_RELEASE, RCON_CmdQuery_f, nullptr);
	ConCommand::Create("rcon_disconnect", "Disconnect from RCON server.", FCVAR_CLIENTDLL | FCVAR_RELEASE, RCON_Disconnect_f, nullptr);
	//-------------------------------------------------------------------------
	// UI DLL                                                                 |
	ConCommand::Create("script_ui", "Run input code as UI script on the VM.", FCVAR_CLIENTDLL | FCVAR_CHEAT, SQVM_UIScript_f, nullptr);
#endif // !DEDICATED
	//-------------------------------------------------------------------------
	// FILESYSTEM API                                                         |
	ConCommand::Create("fs_vpk_mount",  "Mounts a VPK file for FileSystem usage.", FCVAR_DEVELOPMENTONLY, VPK_Mount_f, nullptr);
	ConCommand::Create("fs_vpk_build",  "Builds a VPK file from current workspace.", FCVAR_DEVELOPMENTONLY, VPK_Pack_f, nullptr);
	ConCommand::Create("fs_vpk_unpack", "Unpacks all files from a VPK file.", FCVAR_DEVELOPMENTONLY, VPK_Unpack_f, nullptr);
	//-------------------------------------------------------------------------
	// RTECH API                                                              |
	ConCommand::Create("rtech_strtoguid", "Calculates the GUID from input data.", FCVAR_DEVELOPMENTONLY, RTech_StringToGUID_f, nullptr);
	ConCommand::Create("rtech_decompress", "Decompresses the specified RPAK file.", FCVAR_DEVELOPMENTONLY, RTech_Decompress_f, nullptr);
	ConCommand::Create("pak_requestload", "Requests asynchronous load for specified RPAK file.", FCVAR_DEVELOPMENTONLY, Pak_RequestLoad_f, nullptr);
	ConCommand::Create("pak_requestunload", "Requests unload for specified RPAK file or ID.", FCVAR_DEVELOPMENTONLY, Pak_RequestUnload_f, nullptr);
	ConCommand::Create("pak_swap", "Requests swap for specified RPAK file or ID", FCVAR_DEVELOPMENTONLY, Pak_Swap_f, nullptr);
	ConCommand::Create("pak_listpaks", "Display a list of the loaded Pak files.", FCVAR_RELEASE, Pak_ListPaks_f, nullptr);
	//-------------------------------------------------------------------------
	// NETCHANNEL                                                             |
	ConCommand::Create("net_setkey", "Sets user specified base64 net key.", FCVAR_RELEASE, NET_SetKey_f, nullptr);
	ConCommand::Create("net_generatekey", "Generates and sets a random base64 net key.", FCVAR_RELEASE, NET_GenerateKey_f, nullptr);
}

//-----------------------------------------------------------------------------
// Purpose: shipped ConCommand initialization
//-----------------------------------------------------------------------------
void ConCommand::InitShipped(void)
{
	///------------------------------------------------------ [ CALLBACK SWAP ]
	//-------------------------------------------------------------------------
	// ENGINE DLL                                                             |
	ConCommand* migrateme = g_pCVar->FindCommand("migrateme");
	ConCommand* help = g_pCVar->FindCommand("help");
	ConCommand* changelevel = g_pCVar->FindCommand("changelevel");
	ConCommand* convar_list =  g_pCVar->FindCommand("convar_list");
	ConCommand* convar_differences = g_pCVar->FindCommand("convar_differences");
	ConCommand* convar_findByFlags = g_pCVar->FindCommand("convar_findByFlags");
#ifndef DEDICATED
	//-------------------------------------------------------------------------
	// MATERIAL SYSTEM
	ConCommand* mat_crosshair = g_pCVar->FindCommand("mat_crosshair"); // Patch callback function to working callback.
	mat_crosshair->m_fnCommandCallback = Mat_CrossHair_f;
#endif // !DEDICATED

	help->m_fnCommandCallback = CVHelp_f;
#ifndef CLIENT_DLL
	changelevel->m_fnCommandCallback = Host_Changelevel_f;
#endif // !CLIENT_DLL
	convar_list->m_fnCommandCallback = CVList_f;
	convar_differences->m_fnCommandCallback = CVDiff_f;
	convar_findByFlags->m_fnCommandCallback = CVFlag_f;

	/// ------------------------------------------------------ [ FLAG REMOVAL ]
	//-------------------------------------------------------------------------
	if (!CommandLine()->CheckParm("-devsdk"))
	{
		const char* pszMaskedBases[] =
		{
#ifndef DEDICATED
			"connect",
			"connectAsSpectator",
			"connectWithKey",
			"silentconnect",
			"set",
			"ping",
#endif // !DEDICATED
			"map",
			"map_background",
			"launchplaylist",
			"quit",
			"exit",
			"reload",
			"restart",
			"status",
			"version",
		};

		for (size_t i = 0; i < SDK_ARRAYSIZE(pszMaskedBases); i++)
		{
			if (ConCommandBase* pCommandBase = g_pCVar->FindCommandBase(pszMaskedBases[i]))
			{
				pCommandBase->RemoveFlags(FCVAR_DEVELOPMENTONLY);
			}
		}

		migrateme->RemoveFlags(FCVAR_SERVER_CAN_EXECUTE);
		help->RemoveFlags(FCVAR_DEVELOPMENTONLY);
		changelevel->RemoveFlags(FCVAR_DEVELOPMENTONLY);
		convar_list->RemoveFlags(FCVAR_DEVELOPMENTONLY);
		convar_differences->RemoveFlags(FCVAR_DEVELOPMENTONLY);
		convar_findByFlags->RemoveFlags(FCVAR_DEVELOPMENTONLY);
	}
}

//-----------------------------------------------------------------------------
// Purpose: unregister extraneous ConCommand's.
//-----------------------------------------------------------------------------
void ConCommand::PurgeShipped(void) const
{
#ifdef DEDICATED
	const char* pszCommandToRemove[] =
	{
		"bind",
		"bind_held",
		"bind_list",
		"bind_list_abilities",
		"bind_US_standard",
		"bind_held_US_standard",
		"unbind",
		"unbind_US_standard",
		"unbindall",
		"unbind_all_gamepad",
		"unbindall_ignoreGamepad",
		"unbind_batch",
		"unbind_held",
		"unbind_held_US_standard",
		"uiscript_reset",
		"getpos_bind",
		"connect",
		"silent_connect",
		"ping",
		"gameui_activate",
		"gameui_hide",
		"weaponSelectOrdnance",
		"weaponSelectPrimary0",
		"weaponSelectPrimary1",
		"weaponSelectPrimary2",
		"+scriptCommand1",
		"-scriptCommand1",
		"+scriptCommand2",
		"-scriptCommand2",
		"+scriptCommand3",
		"-scriptCommand3",
		"+scriptCommand4",
		"-scriptCommand4",
		"+scriptCommand5",
		"-scriptCommand5",
		"+scriptCommand6",
		"-scriptCommand6",
		"+scriptCommand7",
		"-scriptCommand7",
		"+scriptCommand8",
		"-scriptCommand8",
		"+scriptCommand9",
		"-scriptCommand9",
	};

	for (size_t i = 0; i < SDK_ARRAYSIZE(pszCommandToRemove); i++)
	{
		ConCommandBase* pCommandBase = g_pCVar->FindCommandBase(pszCommandToRemove[i]);

		if (pCommandBase)
		{
			g_pCVar->UnregisterConCommand(pCommandBase);
		}
	}
#endif // DEDICATED
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this is a command 
// Output : bool
//-----------------------------------------------------------------------------
bool ConCommand::IsCommand(void) const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this is a command 
// Output : bool
//-----------------------------------------------------------------------------
bool ConCommandBase::IsCommand(void) const
{
	return m_pConCommandBaseVFTable != g_pConVarVFTable.RCast<void*>();
}

//-----------------------------------------------------------------------------
// Purpose: Has this cvar been registered
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ConCommandBase::IsRegistered(void) const
{
	return m_bRegistered;
}

//-----------------------------------------------------------------------------
// Purpose: Test each ConCommand query before execution.
// Input  : *pCommandBase - nFlags 
// Output : False if execution is permitted, true if not.
//-----------------------------------------------------------------------------
bool ConCommandBase::IsFlagSetInternal(const ConCommandBase* pCommandBase, int nFlags)
{
	if (cm_unset_all_cmdquery->GetBool())
	{
		// Returning false on all queries may cause problems.
		return false;
	}
	// Default behavior.
	return pCommandBase->HasFlags(nFlags) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Checks if ConCommand has requested flags.
// Input  : nFlags - 
// Output : True if ConCommand has nFlags.
//-----------------------------------------------------------------------------
bool ConCommandBase::HasFlags(int nFlags) const
{
	return m_nFlags & nFlags;
}

//-----------------------------------------------------------------------------
// Purpose: Add's flags to ConCommand.
// Input  : nFlags - 
//-----------------------------------------------------------------------------
void ConCommandBase::AddFlags(int nFlags)
{
	m_nFlags |= nFlags;
}

//-----------------------------------------------------------------------------
// Purpose: Removes flags from ConCommand.
// Input  : nFlags - 
//-----------------------------------------------------------------------------
void ConCommandBase::RemoveFlags(int nFlags)
{
	m_nFlags &= ~nFlags;
}

//-----------------------------------------------------------------------------
// Purpose: Returns current flags.
// Output : int
//-----------------------------------------------------------------------------
int ConCommandBase::GetFlags(void) const
{
	return m_nFlags;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const ConCommandBase
//-----------------------------------------------------------------------------
ConCommandBase* ConCommandBase::GetNext(void) const
{
	return m_pNext;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the ConCommandBase name.
// Output : const char*
//-----------------------------------------------------------------------------
const char* ConCommandBase::GetName(void) const
{
	return m_pszName;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the ConCommandBase help text.
// Output : const char*
//-----------------------------------------------------------------------------
const char* ConCommandBase::GetHelpText(void) const
{
	return m_pszHelpString;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the ConCommandBase usage text.
// Output : const char*
//-----------------------------------------------------------------------------
const char* ConCommandBase::GetUsageText(void) const
{
	return m_pszUsageString;
}

//-----------------------------------------------------------------------------
// Purpose: Copies string using local new/delete operators
// Input  : *szFrom - 
// Output : char
//-----------------------------------------------------------------------------
char* ConCommandBase::CopyString(const char* szFrom) const
{
	size_t nLen;
	char* szTo;

	nLen = strlen(szFrom);
	if (nLen <= 0)
	{
		szTo = new char[1];
		szTo[0] = 0;
	}
	else
	{
		szTo = new char[nLen + 1];
		memmove(szTo, szFrom, nLen + 1);
	}
	return szTo;
}

//-----------------------------------------------------------------------------
// Purpose: Returns current player calling this function
// Output : ECommandTarget_t - 
//-----------------------------------------------------------------------------
ECommandTarget_t Cbuf_GetCurrentPlayer(void)
{
	// Always returns 'CBUF_FIRST_PLAYER' in Respawn's code.
	return ECommandTarget_t::CBUF_FIRST_PLAYER;
}

//-----------------------------------------------------------------------------
// Purpose: Sends the entire command line over to the server
// Input  : *args - 
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool Cmd_ForwardToServer(const CCommand* args)
{
#ifndef DEDICATED
	// Client -> Server command throttling.
	static double flForwardedCommandQuotaStartTime = -1;
	static int nForwardedCommandQuotaCount = 0;

	// No command to forward.
	if (args->ArgC() == 0)
		return false;

	double flStartTime = Plat_FloatTime();
	int nCmdQuotaLimit = cl_quota_stringCmdsPerSecond->GetInt();
	const char* pszCmdString = nullptr;

	// Special case: "cmd whatever args..." is forwarded as "whatever args...";
	// in this case we strip "cmd" from the input.
	if (Q_strcasecmp(args->Arg(0), "cmd") == 0)
		pszCmdString = args->ArgS();
	else
		pszCmdString = args->GetCommandString();

	if (nCmdQuotaLimit)
	{
		if (flStartTime - flForwardedCommandQuotaStartTime >= 1.0)
		{
			flForwardedCommandQuotaStartTime = flStartTime;
			nForwardedCommandQuotaCount = 0;
		}
		++nForwardedCommandQuotaCount;

		if (nForwardedCommandQuotaCount > nCmdQuotaLimit)
		{
			// If we are over quota commands per second, dump this on the floor.
			// If we spam the server with too many commands, it will kick us.
			Warning(eDLL_T::CLIENT, "Command '%s' ignored (submission quota of '%d' per second exceeded!)\n", args->ArgS(), nCmdQuotaLimit);
			return false;
		}
	}
	return v_Cmd_ForwardToServer(args);
#else // !DEDICATED
	return false; // Client only.
#endif // DEDICATED
}

///////////////////////////////////////////////////////////////////////////////
void ConCommand_Attach()
{
	DetourAttach((LPVOID*)&ConCommandBase_IsFlagSet, &ConCommandBase::IsFlagSetInternal);
	DetourAttach((LPVOID*)&v_Cmd_ForwardToServer, &Cmd_ForwardToServer);
}
void ConCommand_Detach()
{
	DetourDetach((LPVOID*)&ConCommandBase_IsFlagSet, &ConCommandBase::IsFlagSetInternal);
	DetourDetach((LPVOID*)&v_Cmd_ForwardToServer, &Cmd_ForwardToServer);
}
ConCommand* g_pConCommand = new ConCommand();
