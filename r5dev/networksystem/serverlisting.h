#pragma once


struct NetGameMod_t
{
	string m_svPackage;
	int m_nNumber;
	bool m_bRequired;
	string m_svDownloadLink;

	//NLOHMANN_DEFINE_TYPE_INTRUSIVE(NetGameMod_t, m_svPackage, m_nNumber, m_bRequired, m_svDownloadLink)
};

struct NetGameServer_t
{
	string m_svHostName;
	string m_svDescription;
	bool m_bHidden;

	string m_svHostMap = "mp_lobby";
	string m_svPlaylist = "dev_default";

	string m_svIpAddress;
	string m_svGamePort;
	string m_svEncryptionKey;

	string m_svRemoteChecksum;
	string m_svSDKVersion;

	string m_svPlayerCount;
	string m_svMaxPlayers;
	int64_t m_nTimeStamp = -1;

	string m_svPublicRef;
	string m_svCachedId;

	//vector<NetGameMod_t> m_vMods;

	static bool CompareHostname (const NetGameServer_t &a, const NetGameServer_t &b) {
		return a.m_svHostName < b.m_svHostName;
	}

	static bool CompareMapname(const NetGameServer_t &a, const NetGameServer_t &b)  {
		return a.m_svHostMap < b.m_svHostMap;
	}

	static bool ComparePlaylist (const NetGameServer_t &a, const NetGameServer_t &b)  {
		return a.m_svPlaylist < b.m_svPlaylist;
	}

	static bool ComparePlayers (const NetGameServer_t &a, const NetGameServer_t &b)  {
		const int playersA = atoi(a.m_svPlayerCount.c_str());
		const int playersB = atoi(b.m_svPlayerCount.c_str());
		return playersA < playersB;
	}

	static bool ComparePort (const NetGameServer_t &a, const NetGameServer_t &b)  {
		return a.m_svGamePort < b.m_svGamePort;
	}
};