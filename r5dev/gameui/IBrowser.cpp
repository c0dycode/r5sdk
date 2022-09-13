/******************************************************************************
-------------------------------------------------------------------------------
File   : IBrowser.cpp
Date   : 09:06:2021
Author : Sal
Purpose: Implements the in-game server browser front-end
-------------------------------------------------------------------------------
History:
- 09:06:2021   21:07 : Created by Sal
- 25:07:2021   14:26 : Implement private servers connect dialog and password field

******************************************************************************/

#include "core/stdafx.h"
#include "core/init.h"
#include "core/resource.h"
#include "tier0/fasttimer.h"
#include "tier0/frametask.h"
#include "tier0/commandline.h"
#include "tier1/cvar.h"
#include "windows/id3dx.h"
#include "windows/console.h"
#include "windows/resource.h"
#include "engine/net.h"
#include "engine/cmodel_bsp.h"
#include "engine/host_state.h"
#ifndef CLIENT_DLL
#include "engine/server/server.h"
#endif // CLIENT_DLL
#include "networksystem/serverlisting.h"
#include "networksystem/pylon.h"
#include "networksystem/listmanager.h"
#include "squirrel/sqinit.h"
#include "vpc/keyvalues.h"
#include "vstdlib/callback.h"
#include "gameui/IBrowser.h"
#include "public/edict.h"
#include "public/utility/utility.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBrowser::CBrowser(void)
{
    memset(m_szServerAddressBuffer, '\0', sizeof(m_szServerAddressBuffer));

    std::thread refresh(&CBrowser::RefreshServerList, this);
    refresh.detach();

    m_pszBrowserTitle = "Server Browser";
    m_rLockedIconBlob = GetModuleResource(IDB_PNG2);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBrowser::~CBrowser(void)
{
    //delete r5net;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBrowser::Init(void)
{
    SetStyleVar();
    m_szMatchmakingHostName = pylon_matchmaking_hostname->GetString();

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: draws the main browser front-end
//-----------------------------------------------------------------------------
void CBrowser::RunFrame(void)
{
    if (!m_bInitialized)
    {
        Init();
        m_bInitialized = true;
    }

    {
        //ImGui::ShowStyleEditor();
        //ImGui::ShowDemoWindow();
    }

    int nVars = 0;
    if (m_Style == ImGuiStyle_t::MODERN)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8.f, 10.f }); nVars++;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_flFadeAlpha);               nVars++;
    }
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 6.f, 6.f });  nVars++;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_flFadeAlpha);               nVars++;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(928.f, 524.f));        nVars++;

    if (m_Style != ImGuiStyle_t::MODERN)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);              nVars++;
    }

    if (!ImGui::Begin(m_pszBrowserTitle, &m_bActivate, ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::End();
        ImGui::PopStyleVar(nVars);
        return;
    }

    ImGui::PopStyleVar(nVars);
    DrawSurface();

    ImGui::End();
}

//-----------------------------------------------------------------------------
// Purpose: runs tasks for the browser while not being drawn 
// (!!! RunTask and RunFrame must be called from the same thread !!!)
//-----------------------------------------------------------------------------
void CBrowser::RunTask()
{
    static bool bInitialized = false;
    static CFastTimer timer;

    if (!bInitialized)
    {
        timer.Start();
        bInitialized = true;
    }

    if (timer.GetDurationInProgress().GetSeconds() > pylon_host_update_interval->GetDouble())
    {
        UpdateHostingStatus();
        timer.Start();
    }
}

//-----------------------------------------------------------------------------
// Purpose: think
//-----------------------------------------------------------------------------
void CBrowser::Think(void)
{
    if (m_bActivate)
    {
        if (m_flFadeAlpha <= 1.f)
        {
            m_flFadeAlpha += .1f;
        }
    }
    else // Reset to full transparent.
    {
        m_flFadeAlpha = 0.f;
    }
}

//-----------------------------------------------------------------------------
// Purpose: draws the compmenu
//-----------------------------------------------------------------------------
void CBrowser::DrawSurface(void)
{
    std::lock_guard<std::mutex> l(m_Mutex);

    ImGui::BeginTabBar("CompMenu");
    if (ImGui::BeginTabItem("Browsing"))
    {
        BrowserPanel();
        ImGui::EndTabItem();
    }
#ifndef CLIENT_DLL
    if (ImGui::BeginTabItem("Hosting"))
    {
        HostPanel();
        ImGui::EndTabItem();
    }
#endif // !CLIENT_DLL
    if (ImGui::BeginTabItem("Settings"))
    {
        SettingsPanel();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}

//-----------------------------------------------------------------------------
// Purpose: draws the server browser section
//-----------------------------------------------------------------------------
void CBrowser::BrowserPanel(void)
{
    ImGui::BeginGroup();
    m_imServerBrowserFilter.Draw();
    ImGui::SameLine();
    if (ImGui::Button("Refresh List"))
    {
        m_svServerListMessage.clear();

        std::thread refresh(&CBrowser::RefreshServerList, this);
        refresh.detach();
    }
    ImGui::EndGroup();
    ImGui::TextColored(ImVec4(1.00f, 0.00f, 0.00f, 1.00f), m_svServerListMessage.c_str());
    ImGui::Separator();

    const float fFooterHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("##ServerBrowser_ServerList", { 0, -fFooterHeight }, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (ImGui::BeginTable("##ServerBrowser_ServerListTable", 6, ImGuiTableFlags_Resizable))
    {
        int nVars = 0;
        if (m_Style == ImGuiStyle_t::MODERN)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 8.f, 0.f }); nVars++;
        }
        else
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.f, 0.f }); nVars++;
        }

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 25);
        ImGui::TableSetupColumn("Map", ImGuiTableColumnFlags_WidthStretch, 20);
        ImGui::TableSetupColumn("Playlist", ImGuiTableColumnFlags_WidthStretch, 10);
        ImGui::TableSetupColumn("Players", ImGuiTableColumnFlags_WidthStretch, 5);
        ImGui::TableSetupColumn("Port", ImGuiTableColumnFlags_WidthStretch, 5);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 5);
        ImGui::TableHeadersRow();

        g_pServerListManager->m_Mutex.lock();
        for (const NetGameServer_t& server : g_pServerListManager->m_vServerList)
        {
            const char* pszHostName = server.m_svHostName.c_str();
            const char* pszHostMap = server.m_svHostMap.c_str();
            const char* pszPlaylist = server.m_svPlaylist.c_str();
            const char* pszHostPort = server.m_svGamePort.c_str();

            if (m_imServerBrowserFilter.PassFilter(pszHostName)
                || m_imServerBrowserFilter.PassFilter(pszHostMap)
                || m_imServerBrowserFilter.PassFilter(pszHostPort))
            {
                ImGui::TableNextColumn();
                ImGui::Text(pszHostName);

                ImGui::TableNextColumn();
                ImGui::Text(pszHostMap);

                ImGui::TableNextColumn();
                ImGui::Text(pszPlaylist);

                ImGui::TableNextColumn();
                ImGui::Text(fmt::format("{:3d}/{:3d}", strtol(server.m_svPlayerCount.c_str(), NULL, NULL), strtol(server.m_svMaxPlayers.c_str(), NULL, NULL)).c_str());

                ImGui::TableNextColumn();
                ImGui::Text(pszHostPort);

                ImGui::TableNextColumn();
                string svConnectBtn = "Connect##";
                svConnectBtn.append(server.m_svHostName + server.m_svIpAddress + server.m_svHostMap);

                if (ImGui::Button(svConnectBtn.c_str()))
                {
                    g_pServerListManager->ConnectToServer(server.m_svIpAddress, pszHostPort, server.m_svEncryptionKey);
                }
            }
        }

        // Sort Serverlist, if sorting was requested by the user
        const auto sortSpecs = ImGui::TableGetSortSpecs();
        if (sortSpecs != nullptr && sortSpecs->SpecsDirty)
        {
            if (sortSpecs->SpecsCount > 0)
            {
                SortServerList(sortSpecs);
                sortSpecs->SpecsDirty = false;
            }
        }

        g_pServerListManager->m_Mutex.unlock();

        ImGui::EndTable();
        ImGui::PopStyleVar(nVars);
    }

    ImGui::EndChild();

    ImGui::Separator();
    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() / 4);
    {
        ImGui::InputTextWithHint("##ServerBrowser_ServerConnString", "Enter ip-address and port", m_szServerAddressBuffer, IM_ARRAYSIZE(m_szServerAddressBuffer));

        ImGui::SameLine();
        ImGui::InputTextWithHint("##ServerBrowser_ServerEncKey", "Enter encryption key", m_szServerEncKeyBuffer, IM_ARRAYSIZE(m_szServerEncKeyBuffer));

        ImGui::SameLine();
        if (ImGui::Button("Connect", ImVec2(ImGui::GetWindowContentRegionWidth() / 4.3f, ImGui::GetFrameHeight())))
        {
            g_pServerListManager->ConnectToServer(m_szServerAddressBuffer, m_szServerEncKeyBuffer);
        }

        ImGui::SameLine();
        if (ImGui::Button("Private Servers", ImVec2(ImGui::GetWindowContentRegionWidth() / 4.3f, ImGui::GetFrameHeight())))
        {
            ImGui::OpenPopup("Private Server");
        }
        HiddenServersModal();
    }
    ImGui::PopItemWidth();
}

//-----------------------------------------------------------------------------
// Purpose: refreshes the server browser list with available servers
//-----------------------------------------------------------------------------
void CBrowser::RefreshServerList(void)
{
    DevMsg(eDLL_T::CLIENT, "Refreshing server list with matchmaking host '%s'\n", pylon_matchmaking_hostname->GetString());
    
    std::string svServerListMessage;
    g_pServerListManager->RefreshServerList(svServerListMessage);

    std::lock_guard<std::mutex> l(m_Mutex);
    m_svServerListMessage = svServerListMessage;
}

//-----------------------------------------------------------------------------
// Purpose: draws the hidden private server modal
//-----------------------------------------------------------------------------
void CBrowser::HiddenServersModal(void)
{
    bool bModalOpen = true;
    if (ImGui::BeginPopupModal("Private Server", &bModalOpen, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetWindowSize(ImVec2(408.f, 204.f), ImGuiCond_Always);

        if (!m_idLockedIcon)
        {
            bool ret = LoadTextureBuffer(reinterpret_cast<unsigned char*>(m_rLockedIconBlob.m_pData), static_cast<int>(m_rLockedIconBlob.m_nSize),
                &m_idLockedIcon, &m_rLockedIconBlob.m_nWidth, &m_rLockedIconBlob.m_nHeight);
            IM_ASSERT(ret);
        }

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f)); // Override the style color for child bg.

        ImGui::BeginChild("##HiddenServersConnectModal_IconParent", ImVec2(m_rLockedIconBlob.m_nWidth, m_rLockedIconBlob.m_nHeight));
        ImGui::Image(m_idLockedIcon, ImVec2(m_rLockedIconBlob.m_nWidth, m_rLockedIconBlob.m_nHeight)); // Display texture.
        ImGui::EndChild();

        ImGui::PopStyleColor(); // Pop the override for the child bg.

        ImGui::SameLine();
        ImGui::Text("Enter token to connect");

        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth()); // Override item width.
        ImGui::InputTextWithHint("##HiddenServersConnectModal_TokenInput", "Token (Required)", &m_svHiddenServerToken);
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(ImGui::GetWindowContentRegionWidth(), 19.f)); // Place a dummy, basically making space inserting a blank element.

        ImGui::TextColored(m_ivHiddenServerMessageColor, m_svHiddenServerRequestMessage.c_str());
        ImGui::Separator();

        if (ImGui::Button("Connect", ImVec2(ImGui::GetWindowContentRegionWidth(), 24)))
        {
            m_svHiddenServerRequestMessage.clear();
            if (!m_svHiddenServerToken.empty())
            {
                NetGameServer_t server;
                bool result = g_pMasterServer->GetServerByToken(server, m_svHiddenServerRequestMessage, m_svHiddenServerToken); // Send token connect request.

                if (!server.m_svHostName.empty())
                {
                    g_pServerListManager->ConnectToServer(server.m_svIpAddress, server.m_svGamePort, server.m_svEncryptionKey); // Connect to the server
                    m_svHiddenServerRequestMessage = "Found Server: " + server.m_svHostName;
                    m_ivHiddenServerMessageColor = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    m_svHiddenServerRequestMessage = "Error: " + m_svHiddenServerRequestMessage;
                    m_ivHiddenServerMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
                }
            }
            else
            {
                m_svHiddenServerRequestMessage = "Token is required." + m_svHiddenServerRequestMessage;
                m_ivHiddenServerMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            }
        }
        if (ImGui::Button("Close", ImVec2(ImGui::GetWindowContentRegionWidth(), 24)))
        {
            m_svHiddenServerRequestMessage.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else if (!bModalOpen)
    {
        m_svHiddenServerRequestMessage.clear();
    }
}

//-----------------------------------------------------------------------------
// Purpose: draws the host section
//-----------------------------------------------------------------------------
void CBrowser::HostPanel(void)
{
#ifndef CLIENT_DLL
    std::lock_guard<std::mutex> l(g_pServerListManager->m_Mutex);

    ImGui::InputTextWithHint("##ServerHost_ServerName", "Server Name (Required)", &g_pServerListManager->m_Server.m_svHostName);
    ImGui::InputTextWithHint("##ServerHost_ServerDesc", "Server Description (Optional)", &g_pServerListManager->m_Server.m_svDescription);
    ImGui::Spacing();

    if (ImGui::BeginCombo("Playlist", g_pServerListManager->m_Server.m_svPlaylist.c_str()))
    {
        g_PlaylistsVecMutex.lock();
        for (const string& svPlaylist : g_vAllPlaylists)
        {
            if (ImGui::Selectable(svPlaylist.c_str(), svPlaylist == g_pServerListManager->m_Server.m_svPlaylist))
            {
                g_pServerListManager->m_Server.m_svPlaylist = svPlaylist;
            }
        }

        g_PlaylistsVecMutex.unlock();
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Map##ServerHost_MapListBox", g_pServerListManager->m_Server.m_svHostMap.c_str()))
    {
        g_MapVecMutex.lock();
        for (const string& svMap : g_vAllMaps)
        {
            if (ImGui::Selectable(svMap.c_str(), svMap == g_pServerListManager->m_Server.m_svHostMap))
            {
                g_pServerListManager->m_Server.m_svHostMap = svMap;
            }
        }

        g_MapVecMutex.unlock();
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Load Global Ban List", &g_bCheckCompBanDB);
    ImGui::Spacing();

    ImGui::SameLine();
    ImGui::Text("Server Visibility");

    if (ImGui::SameLine(); ImGui::RadioButton("Offline", g_pServerListManager->m_ServerVisibility == EServerVisibility_t::OFFLINE))
    {
        g_pServerListManager->m_ServerVisibility = EServerVisibility_t::OFFLINE;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("Hidden", g_pServerListManager->m_ServerVisibility == EServerVisibility_t::HIDDEN))
    {
        g_pServerListManager->m_ServerVisibility = EServerVisibility_t::HIDDEN;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("Public", g_pServerListManager->m_ServerVisibility == EServerVisibility_t::PUBLIC))
    {
        g_pServerListManager->m_ServerVisibility = EServerVisibility_t::PUBLIC;
    }

    ImGui::TextColored(m_HostRequestMessageColor, m_svHostRequestMessage.c_str());
    if (!m_svHostToken.empty())
    {
        ImGui::InputText("##ServerHost_HostToken", &m_svHostToken, ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::Spacing();
    if (!g_pHostState->m_bActiveGame)
    {
        if (ImGui::Button("Start Server", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            m_svHostRequestMessage.clear();

            bool bEnforceField = g_pServerListManager->m_ServerVisibility == EServerVisibility_t::OFFLINE ? true : !g_pServerListManager->m_Server.m_svHostName.empty();
            if (bEnforceField && !g_pServerListManager->m_Server.m_svPlaylist.empty() && !g_pServerListManager->m_Server.m_svHostMap.empty())
            {
                g_pServerListManager->LaunchServer(); // Launch server.
            }
            else
            {
                if (g_pServerListManager->m_Server.m_svHostName.empty())
                {
                    m_svHostRequestMessage = "Server name is required.";
                    m_HostRequestMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
                }
                else if (g_pServerListManager->m_Server.m_svPlaylist.empty())
                {
                    m_svHostRequestMessage = "Playlist is required.";
                    m_HostRequestMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
                }
                else if (g_pServerListManager->m_Server.m_svHostMap.empty())
                {
                    m_svHostRequestMessage = "Level name is required.";
                    m_HostRequestMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
                }
            }
        }
        if (ImGui::Button("Reload Playlist", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            g_TaskScheduler->Dispatch([]()
                {
                    _DownloadPlaylists_f();
                    KeyValues::InitPlaylists(); // Re-Init playlist.
                }, 0);
        }

        if (ImGui::Button("Reload Banlist", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            g_TaskScheduler->Dispatch([]()
                {
                    g_pBanSystem->Load();
                }, 0);
        }
    }
    else
    {
        if (ImGui::Button("Stop Server", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            ProcessCommand("LeaveMatch"); // TODO: use script callback instead.
            g_TaskScheduler->Dispatch([]()
                {
                    // Force CHostState::FrameUpdate to shutdown the server for dedicated.
                    g_pHostState->m_iNextState = HostStates_t::HS_GAME_SHUTDOWN;
                }, 0);
        }

        if (ImGui::Button("Change Level", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            if (!g_pServerListManager->m_Server.m_svHostMap.empty())
            {
                g_pServerListManager->LaunchServer();
            }
            else
            {
                m_svHostRequestMessage = "Failed to change level: 'levelname' was empty.";
                m_HostRequestMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            }
        }

        if (ImGui::Button("Weapon Reparse", ImVec2(ImGui::GetWindowContentRegionWidth(), 32)))
        {
            DevMsg(eDLL_T::ENGINE, "Reparsing weapon data on %s\n", "server and client");
            ProcessCommand("weapon_reparse");
        }
    }
#endif // !CLIENT_DLL
}

//-----------------------------------------------------------------------------
// Purpose: updates the host status
//-----------------------------------------------------------------------------
void CBrowser::UpdateHostingStatus(void)
{
#ifndef CLIENT_DLL
    if (!g_pHostState || !g_pCVar)
    {
        return;
    }

    std::lock_guard<std::mutex> l(g_pServerListManager->m_Mutex);
    g_pServerListManager->m_HostingStatus = g_pServer->IsActive() ? EHostStatus_t::HOSTING : EHostStatus_t::NOT_HOSTING; // Are we hosting a server?

    switch (g_pServerListManager->m_HostingStatus)
    {
    case EHostStatus_t::NOT_HOSTING:
    {
        m_svHostToken.clear();

        if (ImGui::ColorConvertFloat4ToU32(m_HostRequestMessageColor) == // Only clear if this is green (a valid hosting message).
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 1.00f, 0.00f, 1.00f)))
        {
            m_svHostRequestMessage.clear();
            m_HostRequestMessageColor = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        }

        break;
    }
    case EHostStatus_t::HOSTING:
    {
        if (g_pServerListManager->m_ServerVisibility == EServerVisibility_t::OFFLINE)
        {
            break;
        }

        if (*g_nClientRemoteChecksum == NULL) // Check if script checksum is valid yet.
        {
            break;
        }

        switch (g_pServerListManager->m_ServerVisibility)
        {

        case EServerVisibility_t::HIDDEN:
            g_pServerListManager->m_Server.m_bHidden = true;
            break;
        case EServerVisibility_t::PUBLIC:
            g_pServerListManager->m_Server.m_bHidden = false;
            break;
        default:
            break;
        }

        g_NetKeyMutex.lock();
        NetGameServer_t netGameServer // !FIXME: create from main thread.
        {
            g_pServerListManager->m_Server.m_svHostName,
            g_pServerListManager->m_Server.m_svDescription,
            g_pServerListManager->m_Server.m_bHidden,
            g_pHostState->m_levelName,
            mp_gamemode->GetString(),
            hostip->GetString(),
            hostport->GetString(),
            g_svNetKey,
            std::to_string(*g_nServerRemoteChecksum),
            SDK_VERSION,
            std::to_string(g_pServer->GetNumHumanPlayers() + g_pServer->GetNumFakeClients()),
            std::to_string(g_ServerGlobalVariables->m_nMaxClients),
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count()
        };
        g_NetKeyMutex.unlock();

        std::thread post(&CBrowser::SendHostingPostRequest, this, netGameServer);
        post.detach();

        break;
    }
    default:
        break;
    }
#endif // !CLIENT_DLL
}

//-----------------------------------------------------------------------------
// Purpose: sends the hosting POST request to the comp server
// Input  : &gameServer - 
//-----------------------------------------------------------------------------
void CBrowser::SendHostingPostRequest(const NetGameServer_t& gameServer)
{
#ifndef CLIENT_DLL
    string svHostRequestMessage;
    string svHostToken;
    bool result = g_pMasterServer->PostServerHost(svHostRequestMessage, svHostToken, gameServer);

    std::lock_guard<std::mutex> l(m_Mutex);

    m_svHostRequestMessage = svHostRequestMessage;
    m_svHostToken = svHostToken;

    if (result)
    {
        m_HostRequestMessageColor = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
        stringstream ssMessage;
        ssMessage << "Broadcasting: ";
        if (!m_svHostToken.empty())
        {
            ssMessage << "share the following token for clients to connect: ";
        }
        m_svHostRequestMessage = ssMessage.str();
    }
    else
    {
        m_HostRequestMessageColor = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    }
#endif // !CLIENT_DLL
}

//-----------------------------------------------------------------------------
// Purpose: processes submitted commands for the main thread
// Input  : *pszCommand - 
//-----------------------------------------------------------------------------
void CBrowser::ProcessCommand(const char* pszCommand) const
{
    Cbuf_AddText(Cbuf_GetCurrentPlayer(), pszCommand, cmd_source_t::kCommandSrcCode);
    //g_TaskScheduler->Dispatch(Cbuf_Execute, 0); // Run in main thread.
}

//-----------------------------------------------------------------------------
// Purpose: draws the settings section
//-----------------------------------------------------------------------------
void CBrowser::SettingsPanel(void)
{
    ImGui::InputTextWithHint("Hostname", "Matchmaking Server String", &m_szMatchmakingHostName);
    if (ImGui::Button("Update Hostname"))
    {
        ProcessCommand(fmt::format("{:s} \"{:s}\"", "pylon_matchmaking_hostname", m_szMatchmakingHostName).c_str());
    }

    std::lock_guard<std::mutex> l(g_NetKeyMutex);
    ImGui::InputText("Netkey", const_cast<char*>(g_svNetKey.c_str()), ImGuiInputTextFlags_ReadOnly);
    if (ImGui::Button("Regenerate Encryption Key"))
    {
        g_TaskScheduler->Dispatch(NET_GenerateKey, 0);
    }
}

//-----------------------------------------------------------------------------
// Purpose: hooked to 'MP_HostName_Changed_f' to sync hostname field with 
// the 'pylon_matchmaking_hostname' ConVar (!!! DO NOT USE !!!).
// Input  : *pszHostName - 
//-----------------------------------------------------------------------------
void CBrowser::SetHostName(const char* pszHostName)
{
    std::lock_guard<std::mutex> l(m_Mutex);
    m_szMatchmakingHostName = pszHostName;
}

//-----------------------------------------------------------------------------
// Purpose: sets the browser front-end style
//-----------------------------------------------------------------------------
void CBrowser::SetStyleVar(void)
{
    m_Style = g_pImGuiConfig->InitStyle();

    ImGui::SetNextWindowSize(ImVec2(928.f, 524.f), ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(-500.f, 50.f), ImGuiCond_FirstUseEver);
}

void CBrowser::SortServerList(ImGuiTableSortSpecs* pSortSpecs) const
{
    auto serverList = g_pServerListManager->m_vServerList;
    switch (pSortSpecs->Specs->ColumnIndex)
    {
    // Name
    case 0:
        if (pSortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending)
            std::sort(serverList.begin(), serverList.end(), CompareHostname);
        else
            std::sort(serverList.rbegin(), serverList.rend(), CompareHostname);
        break;
    // Map
    case 1:
        if (pSortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending)
            std::sort(serverList.begin(), serverList.end(), CompareMapname);
        else
            std::sort(serverList.rbegin(), serverList.rend(), CompareMapname);
        break;
    // Playlist
    case 2:
        if (pSortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending)
            std::sort(serverList.begin(), serverList.end(), CompareHostname);
        else
            std::sort(serverList.rbegin(), serverList.rend(), CompareHostname);
        break;
    //Players
    case 3:
        if (pSortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending)
            std::sort(serverList.begin(), serverList.end(), ComparePlayers);
        else
            std::sort(serverList.rbegin(), serverList.rend(), ComparePlayers);
        break;
    // Port
    case 4:
        if (pSortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending)
            std::sort(serverList.begin(), serverList.end(), ComparePort);
        else
            std::sort(serverList.rbegin(), serverList.rend(), ComparePort);
        break;
    default:
        break;
    }
    g_pServerListManager->m_vServerList = serverList;
}


CBrowser* g_pBrowser = new CBrowser();