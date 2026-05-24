#include "../main.h"
#include "../game/game.h"
#include "../game/crosshair.h"
#include "../game/CRadarRect.h"
#include "netgame.h"
#include "../gui/gui.h"

extern CWidgetManager* g_pWidgetManager;
extern CCrossHair *pCrossHair;

#include "../chatwindow.h"

#include "..//CClientInfo.h"
#include "..//CLocalisation.h"

// voice
#include "voice/MicroIcon.h"
#include "voice/SpeakerList.h"
#include "voice/Network.h"



#define NETGAME_VERSION 4057
#define AUTH_BS OBFUSCATE("E02262CF28BC542486C558D4BE9EFB716592AFAF8B")
#define LAUNCHER_NAME OBFUSCATE("MOBILE-MOBILE")

extern CGame *pGame;
extern CGUI *pGUI;
extern CChatWindow *pChatWindow;

extern CNetGame *pNetGame;


int iVehiclePoolProcessFlag = 0;
int iPickupPoolProcessFlag = 0;

void RegisterRPCs(RakClientInterface* pRakClient);
void UnRegisterRPCs(RakClientInterface* pRakClient);
void RegisterScriptRPCs(RakClientInterface* pRakClient);
void UnRegisterScriptRPCs(RakClientInterface* pRakClient);

unsigned char GetPacketID(Packet *p)
{
	if(p == 0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	else
		return (unsigned char) p->data[0];
}

CNetGame::CNetGame(const char* szHostOrIp, int iPort, const char* szPlayerName, const char* szPass)
{
	strcpy(m_szHostName, OBFUSCATE("San Andreas Multiplayer"));
	strncpy(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	m_iPort = iPort;

	// voice
	Network::OnRaknetConnect(szHostOrIp, iPort);

	m_pPlayerPool = new CPlayerPool();
	m_pPlayerPool->SetLocalPlayerName(szPlayerName);
	
	m_pVehiclePool = new CVehiclePool();
	m_pObjectPool = new CObjectPool();
	m_pPickupPool = new CPickupPool();
	m_pGangZonePool = new CGangZonePool();
	m_pLabelPool = new CText3DLabelsPool();
	m_pTextDrawPool = new CTextDrawPool();
	g_pWidgetManager = new CWidgetManager();
	//m_pStreamPool = new CStreamPool();
	m_pActorPool = new CActorPool();

	m_pRakClient = RakNetworkFactory::GetRakClientInterface();
	RegisterRPCs(m_pRakClient);
	RegisterScriptRPCs(m_pRakClient);
	m_pRakClient->SetPassword(szPass);

	m_dwLastConnectAttempt = GetTickCount();
	m_iGameState = 	GAMESTATE_WAIT_CONNECT;

	m_iSpawnsAvailable = 0;
	m_bHoldTime = true;
	m_byteWorldMinute = 0;
	m_byteWorldTime = 12;
	m_byteWeather =	10;
	m_fGravity = (float)0.008000000;
	m_bUseCJWalk = false;
	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 60.0f;
	m_bZoneNames = false;
	m_bInstagib = false;
	m_iDeathDropMoney = 0;
	m_bNameTagLOS = false;

	for(int i=0; i<100; i++)
		m_dwMapIcons[i] = 0;

	pGame->EnableClock(false);
	pGame->EnableZoneNames(false);

	if(pChatWindow)
		pChatWindow->AddDebugMessage(OBFUSCATE("{FFFFFF}SA-MP {0080FE}" SAMP_VERSION " {FFFFFF}Started"));
}

CNetGame::~CNetGame()
{
	m_pRakClient->Disconnect(0);

	// voice
	Network::OnRaknetDisconnect();

	UnRegisterRPCs(m_pRakClient);
	UnRegisterScriptRPCs(m_pRakClient);

	RakNetworkFactory::DestroyRakClientInterface(m_pRakClient);

	if(m_pPlayerPool) 
	{
		delete m_pPlayerPool;
		m_pPlayerPool = nullptr;
	}

	if(m_pVehiclePool)
	{
		delete m_pVehiclePool;
		m_pVehiclePool = nullptr;
	}

	if(m_pPickupPool)
	{
		delete m_pPickupPool;
		m_pPickupPool = nullptr;
	}

	if(m_pGangZonePool)
	{
		delete m_pGangZonePool;
		m_pGangZonePool = nullptr;
	}

	if(m_pLabelPool)
	{
		delete m_pLabelPool;
		m_pLabelPool = nullptr;
	}

	if (m_pTextDrawPool)
	{
		delete m_pTextDrawPool;
		m_pTextDrawPool = nullptr;
	}

	if (g_pWidgetManager)
	{
		delete g_pWidgetManager;
		g_pWidgetManager = nullptr;
	}

	/*if (m_pStreamPool)
	{
		delete m_pStreamPool;
		m_pStreamPool = nullptr;
	}*/
}

static bool once = false;
void CNetGame::Process()
{
	// todo: 30 fps fixed rate
	//static uint32_t time = GetTickCount();
	bool bProcess = false;
	UpdateNetwork();
	bProcess = true;
	/*if (GetTickCount() - time >= 1000 / 30)
	{
		UpdateNetwork();
		time = GetTickCount();
		bProcess = true;
	}*/

	// server checkpoints update
	if (m_pPlayerPool)
	{
		if (m_pPlayerPool->GetLocalPlayer())
		{
			if (m_pPlayerPool->GetLocalPlayer()->m_bIsActive && m_pPlayerPool->GetLocalPlayer()->GetPlayerPed())
				pGame->UpdateCheckpoints();
		}
	}

	if(m_bHoldTime)
		pGame->SetWorldTime(m_byteWorldTime, m_byteWorldMinute);

	pGame->PreloadObjectsAnim();

	if(GetGameState() == GAMESTATE_CONNECTED)
	{
		// pool process
		if(m_pPlayerPool && bProcess) m_pPlayerPool->Process();
		if (m_pObjectPool) m_pObjectPool->Process();
		if(m_pVehiclePool && iVehiclePoolProcessFlag > 5)
		{
			m_pVehiclePool->Process();
			iVehiclePoolProcessFlag = 0;
		}
		else
		{
			++iVehiclePoolProcessFlag;
		}

		if(m_pPickupPool && iPickupPoolProcessFlag > 5) 
		{
			m_pPickupPool->Process();
			iPickupPoolProcessFlag = 0;
		}
		else
		{
			//++iPickupPoolProcessFlag;
		}
	}
	else
	{
		CPlayerPed *pPlayer = pGame->FindPlayerPed();
		CCamera *pCamera = pGame->GetCamera();

		if(pPlayer && pCamera)
		{
			if(pPlayer->IsInVehicle())
				pPlayer->RemoveFromVehicleAndPutAt(1093.4f, -2036.5f, 82.7106f);
			else pPlayer->TeleportTo(1093.4f, -2036.5f, 82.7106f);

			pCamera->SetPosition(1093.0f, -2036.0f, 90.0f, 0.0f, 0.0f, 0.0f);
			pCamera->LookAtPoint(384.0f, -1557.0f, 20.0f, 2);

			pGame->SetWorldWeather(m_byteWeather);
			pGame->DisplayWidgets(false);
		}
	}

	if(GetGameState() == GAMESTATE_WAIT_CONNECT && (GetTickCount() - m_dwLastConnectAttempt) > 3000)
	{
		if(pChatWindow)
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTING));

		m_pRakClient->Connect(m_szHostOrIp, m_iPort, 0, 0, 5);
		m_dwLastConnectAttempt = GetTickCount();

		SetGameState(GAMESTATE_CONNECTING);
	}
}

void CNetGame::UpdateNetwork()
{
	Packet* pkt = nullptr;
	unsigned char packetIdentifier;

	while(pkt = m_pRakClient->Receive())
	{
		packetIdentifier = GetPacketID(pkt);

		switch(packetIdentifier)
		{
			case ID_AUTH_KEY:
			Log(OBFUSCATE("Incoming packet: ID_AUTH_KEY"));
			Packet_AuthKey(pkt);
			break;

			case ID_CONNECTION_BANNED:
			case ID_CONNECTION_ATTEMPT_FAILED:
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTION_ATTEMPT_FAILED));
			SetGameState(GAMESTATE_WAIT_CONNECT);

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
			pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::FULL_SERVER));
			SetGameState(GAMESTATE_WAIT_CONNECT);

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_DISCONNECTION_NOTIFICATION:
			Packet_DisconnectionNotification(pkt);

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_CONNECTION_LOST:
			Packet_ConnectionLost(pkt);

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);

			// voice
			MicroIcon::Show();
			SpeakerList::Show();
			break;

			case ID_FAILED_INITIALIZE_ENCRIPTION:
			pChatWindow->AddDebugMessage(OBFUSCATE("Failed to initialize encryption."));

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_INVALID_PASSWORD:
			pChatWindow->AddDebugMessage(OBFUSCATE("Wrong server password."));
			m_pRakClient->Disconnect(0);

			// voice
			MicroIcon::Hide();
			SpeakerList::Hide();
			break;

			case ID_PLAYER_SYNC:
			Packet_PlayerSync(pkt);
			break;

			case ID_VEHICLE_SYNC:
			Packet_VehicleSync(pkt);
			break;

			case ID_PASSENGER_SYNC:
			Packet_PassengerSync(pkt);
			break;

			case ID_MARKERS_SYNC:
			Packet_MarkersSync(pkt);
			break;

			case ID_AIM_SYNC:
			Packet_AimSync(pkt);
			break;

			case ID_BULLET_SYNC:
			Packet_BulletSync(pkt);
			break;

			case ID_TRAILER_SYNC:
			Packet_TrailerSync(pkt);
			break;

			case ID_CUSTOM_RPC:
			Packet_CustomRPC(pkt);
			break;
		}

		bool breakStatus = false;
		if(!Network::OnRaknetReceive(*pkt)) breakStatus = true;
		if(breakStatus) return;

		m_pRakClient->DeallocatePacket(pkt);
	}
}

void CNetGame::Packet_TrailerSync(Packet* p)
{
	CRemotePlayer* pPlayer;
	RakNet::BitStream bsSpectatorSync((unsigned char*)p->data, p->length, false);

	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	BYTE bytePacketID = 0;
	BYTE bytePlayerID = 0;

	TRAILER_SYNC_DATA trSync;

	bsSpectatorSync.Read(bytePacketID);
	bsSpectatorSync.Read(bytePlayerID);
	bsSpectatorSync.Read((char*)& trSync, sizeof(TRAILER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if (pPlayer)
		pPlayer->StoreTrailerFullSyncData(&trSync);
}

void CNetGame::ResetVehiclePool()
{
	Log(OBFUSCATE("ResetVehiclePool"));
	if(m_pVehiclePool)
		delete m_pVehiclePool;

	m_pVehiclePool = new CVehiclePool();
}

void CNetGame::ResetObjectPool()
{
	Log(OBFUSCATE("ResetObjectPool"));
	if(m_pObjectPool)
		delete m_pObjectPool;

	m_pObjectPool = new CObjectPool();
}

void CNetGame::ResetPickupPool()
{
	Log(OBFUSCATE("ResetPickupPool"));
	if(m_pPickupPool)
		delete m_pPickupPool;

	m_pPickupPool = new CPickupPool();
}

void CNetGame::ResetGangZonePool()
{
	Log(OBFUSCATE("ResetGangZonePool"));
	if(m_pGangZonePool)
		delete m_pGangZonePool;

	m_pGangZonePool = new CGangZonePool();
}

void CNetGame::ResetLabelPool()
{
	Log(OBFUSCATE("ResetLabelPool"));
	if(m_pLabelPool)
		delete m_pLabelPool;

	m_pLabelPool = new CText3DLabelsPool();
}

void CNetGame::ResetActorPool()
{
	Log(OBFUSCATE("ResetActorPool"));
	if (m_pActorPool)
		delete m_pActorPool;

	m_pActorPool = new CActorPool();
}

void CNetGame::ResetTextDrawPool()
{
	Log(OBFUSCATE("ResetTextDrawPool"));
	if (m_pTextDrawPool)
		delete m_pTextDrawPool;

	m_pTextDrawPool = new CTextDrawPool();
}

extern int RemoveModelIDs[1200];
extern VECTOR RemovePos[1200];
extern float RemoveRad[1200];
extern int iTotalRemovedObjects;

void CNetGame::ShutDownForGameRestart()
{
	iTotalRemovedObjects = 0;
	for (int & RemoveModelID : RemoveModelIDs)
		RemoveModelID = -1;

	// voice
	SpeakerList::Hide();
    MicroIcon::Hide();
	Network::OnRaknetDisconnect();

	for(PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		CRemotePlayer* pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
		{
			//pPlayer->SetTeam(NO_TEAM);
			//pPlayer->ResetAllSyncAttributes();
		}
	}

	CLocalPlayer *pLocalPlayer = m_pPlayerPool->GetLocalPlayer();
	if(pLocalPlayer)
	{
		pLocalPlayer->ResetAllSyncAttributes();
		pLocalPlayer->ToggleSpectating(false);
	}

	m_iGameState = GAMESTATE_RESTARTING;

	//m_pPlayerPool->DeactivateAll();
	m_pPlayerPool->Process();

	ResetVehiclePool();
	ResetObjectPool();
	ResetPickupPool();
	ResetGangZonePool();
	ResetLabelPool();
	ResetTextDrawPool();
	ResetActorPool();

	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 60.0f;
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather = 1;
	m_bHoldTime = true;
	m_bNameTagLOS = true;
	m_bUseCJWalk = false;
	m_fGravity = 0.008f;
	m_iDeathDropMoney = 0;

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if(pPlayerPed)
	{
		pPlayerPed->SetInterior(0);
		//pPlayerPed->SetDead();
		pPlayerPed->SetArmour(0.0f);
	}

	pGame->ToggleCJWalk(false);
	pGame->ResetLocalMoney();
	pGame->EnableZoneNames(false);
	m_bZoneNames = false;
	GameResetRadarColors();
	pGame->SetGravity(m_fGravity);
	pGame->EnableClock(false);
}

void CNetGame::SendChatMessage(const char* szMsg)
{
	if (GetGameState() != GAMESTATE_CONNECTED) return;

	RakNet::BitStream bsSend;
	uint8_t byteTextLen = strlen(szMsg);

	bsSend.Write(byteTextLen);
	bsSend.Write(szMsg, byteTextLen);

	m_pRakClient->RPC(&RPC_Chat,&bsSend,HIGH_PRIORITY,RELIABLE,0,false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SendChatCommand(const char* szCommand)
{
	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsParams;
	int iStrlen = strlen(szCommand);

	bsParams.Write(iStrlen);
	bsParams.Write(szCommand, iStrlen);
	m_pRakClient->RPC(&RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SendDialogResponse(uint16_t wDialogID, uint8_t byteButtonID, uint16_t wListBoxItem, char* szInput)
{
	uint8_t respLen = strlen(szInput);

	RakNet::BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(byteButtonID);
	bsSend.Write(wListBoxItem);
	bsSend.Write(respLen);
	bsSend.Write(szInput, respLen);
	m_pRakClient->RPC(&RPC_DialogResponse, &bsSend, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SetMapIcon(uint8_t byteIndex, float fX, float fY, float fZ, uint8_t byteIcon, int iColor, int style)
{
	if(byteIndex >= 100)
		return;

	if(m_dwMapIcons[byteIndex]) DisableMapIcon(byteIndex);

	m_dwMapIcons[byteIndex] = pGame->CreateRadarMarkerIcon(byteIcon, fX, fY, fZ, iColor, style);
}

void CNetGame::DisableMapIcon(uint8_t byteIndex)
{
	if(byteIndex >= 100)
		return;

	ScriptCommand(&disable_marker, m_dwMapIcons[byteIndex]);
	m_dwMapIcons[byteIndex] = 0;
}

void CNetGame::UpdatePlayerScoresAndPings()
{
	static uint32_t dwLastUpdateTick = 0;

	if ((GetTickCount() - dwLastUpdateTick) >= 3000)
	{
		dwLastUpdateTick = GetTickCount();
		RakNet::BitStream bsParams;
		m_pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void gen_auth_key(char buf[260], char* auth_in);
void CNetGame::Packet_AuthKey(Packet* pkt)
{
	RakNet::BitStream bsAuth((unsigned char *)pkt->data, pkt->length, false);

	uint8_t byteAuthLen;
	char szAuth[260];

	bsAuth.IgnoreBits(8);
	bsAuth.Read(byteAuthLen);
	bsAuth.Read(szAuth, byteAuthLen);
	szAuth[byteAuthLen] = '\0';

	char szAuthKey[260];
	gen_auth_key(szAuthKey, szAuth);

	RakNet::BitStream bsKey;
	auto byteAuthKeyLen = (uint8_t)strlen(szAuthKey);

	bsKey.Write((uint8_t)ID_AUTH_KEY);
	bsKey.Write((uint8_t)byteAuthKeyLen);
	bsKey.Write(szAuthKey, byteAuthKeyLen);
	m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);

	//Log(OBFUSCATE("[AUTH] %s -> %s"), szAuth, szAuthKey);
}

void CNetGame::Packet_DisconnectionNotification(Packet* pkt)
{
	if(pChatWindow)
		pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::SERVER_CLOSED_CONNECTION));

	m_pRakClient->Disconnect(2000);
}

void CNetGame::Packet_ConnectionLost(Packet* pkt)
{
	if(m_pRakClient) m_pRakClient->Disconnect(0);

	if(pChatWindow)
		pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTION_LOST));

	ShutDownForGameRestart();

	for(PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		CRemotePlayer *pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer) m_pPlayerPool->Delete(playerId, 0);
	}

	SetGameState(GAMESTATE_WAIT_CONNECT);
}

#include "..//CServerManager.h"
bool g_isValidSum(int a)
{
	for (const auto & g_sEncryptedAddresse : g_sEncryptedAddresses)
		if (g_sEncryptedAddresse.getSum() == a) return true;

	return true;
}
void WriteVerified1();
void CNetGame::Packet_ConnectionSucceeded(Packet* pkt)
{
	if(pChatWindow)
		pChatWindow->AddDebugMessageNonFormatted(CLocalisation::GetMsg(E_MSG::CONNECTED));
	SetGameState(GAMESTATE_AWAIT_JOIN);

	RakNet::BitStream bsSuccAuth((unsigned char *)pkt->data, pkt->length, false);
	PLAYERID MyPlayerID;
	unsigned int uiChallenge;

	bsSuccAuth.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
	bsSuccAuth.IgnoreBits(32); // binaryAddress
	bsSuccAuth.IgnoreBits(16); // port
	bsSuccAuth.Read(MyPlayerID);
	bsSuccAuth.Read(uiChallenge);

	char ip[0x7F];
	strncpy(ip, m_szHostOrIp, sizeof(ip));

	std::vector<std::string> strings;
	std::istringstream f((const char*)&ip[0]);
	std::string s;
	int sum = 0;
	while (getline(f, s, '.'))
		sum += std::atoi(s.c_str());

	if (g_isValidSum(sum))
		WriteVerified1();

	m_pPlayerPool->SetLocalPlayerID(MyPlayerID);

	int iVersion = NETGAME_VERSION;
	char byteMod = 0x01;
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;

	char byteAuthBSLen = (char)strlen(AUTH_BS);
	char byteNameLen = (char)strlen(m_pPlayerPool->GetLocalPlayerName());
	char byteClientverLen = (char)strlen(SAMP_VERSION);

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(m_pPlayerPool->GetLocalPlayerName(), byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write(AUTH_BS, byteAuthBSLen);
	bsSend.Write(byteClientverLen);
	bsSend.Write(SAMP_VERSION, byteClientverLen);

	//CClientInfo::WriteClientInfoToBitStream(bsSend);

	// voice
    Network::OnRaknetRpc(RPC_ClientJoin, bsSend);

	m_pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	
	// Custom Packet
	RakNet::BitStream bsParams;
	int8_t iVersionMobile = 1;
	int8_t launcher = 1;
	uint8_t byteLauncherLen = strlen(LAUNCHER_NAME);
	bsParams.Write(launcher);
	bsParams.Write(iVersionMobile);
	bsParams.Write(byteLauncherLen);
	bsParams.Write(LAUNCHER_NAME, byteLauncherLen);
	m_pRakClient->RPC(&RPC_CustomJoin, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SendDonateCar(int carid, int carcost)
{
			RakNet::BitStream bsSend;
            bsSend.Write((uint8_t)  0x41);
         	bsSend.Write((uint32_t) 0x41);
			bsSend.Write((int) carid);
			bsSend.Write((int) carcost);

            pNetGame->GetRakClient()->Send(&bsSend, MEDIUM_PRIORITY, RELIABLE, 0);
}

void CNetGame::Packet_PlayerSync(Packet* pkt)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPlayerSync((unsigned char *)pkt->data, pkt->length, false);
	ONFOOT_SYNC_DATA ofSync;
	uint8_t bytePacketID=0;
	PLAYERID playerId;

	bool bHasLR,bHasUD;
	bool bHasVehicleSurfingInfo;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&ofSync, 0, sizeof(ONFOOT_SYNC_DATA));

	bsPlayerSync.Read(bytePacketID);
	bsPlayerSync.Read(playerId);

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if(bHasLR) bsPlayerSync.Read(ofSync.lrAnalog);

	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if(bHasUD) bsPlayerSync.Read(ofSync.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(ofSync.wKeys);

	// VECTOR POS
	bsPlayerSync.Read((char*)&ofSync.vecPos,sizeof(VECTOR));

	// QUATERNION
	float tw, tx, ty, tz;
	bsPlayerSync.ReadNormQuat(tw, tx, ty, tz);
	ofSync.quat.w = tw;
	ofSync.quat.x = tx;
	ofSync.quat.y = ty;
	ofSync.quat.z = tz;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	uint8_t byteHealthArmour;
	uint8_t byteArmTemp=0,byteHlTemp=0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) ofSync.byteArmour = 100;
	else if(byteArmTemp == 0) ofSync.byteArmour = 0;
	else ofSync.byteArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) ofSync.byteHealth = 100;
	else if(byteHlTemp == 0) ofSync.byteHealth = 0;
	else ofSync.byteHealth = byteHlTemp * 7;

	// CURRENT WEAPON
    bsPlayerSync.Read(ofSync.byteCurrentWeapon);
    // SPECIAL ACTION
    bsPlayerSync.Read(ofSync.byteSpecialAction);

    // READ MOVESPEED VECTORS
    bsPlayerSync.ReadVector(tx, ty, tz);
    ofSync.vecMoveSpeed.X = tx;
    ofSync.vecMoveSpeed.Y = ty;
    ofSync.vecMoveSpeed.Z = tz;

    bsPlayerSync.Read(bHasVehicleSurfingInfo);
    if (bHasVehicleSurfingInfo)
    {
        bsPlayerSync.Read(ofSync.wSurfInfo);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.X);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.Y);
        bsPlayerSync.Read(ofSync.vecSurfOffsets.Z);
    }
    else
        ofSync.wSurfInfo = INVALID_VEHICLE_ID;

	bool bHasAnimInfo;
	bsPlayerSync.Read(bHasAnimInfo);

	if (bHasAnimInfo)
		bsPlayerSync.Read(ofSync.dwAnimation);
	else ofSync.dwAnimation = 0b10000000000000000000000000000000;

	uint8_t key = 0;

    if(m_pPlayerPool)
    {
    	pPlayer = m_pPlayerPool->GetAt(playerId);
    	if(pPlayer)
    		pPlayer->StoreOnFootFullSyncData(&ofSync, 0, key);
    }
}

void CNetGame::Packet_VehicleSync(Packet* pkt)
{
	CRemotePlayer *pPlayer;
	RakNet::BitStream bsSync((unsigned char *)pkt->data, pkt->length, false);
	uint8_t bytePacketID = 0;
	PLAYERID playerId;
	INCAR_SYNC_DATA icSync;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

	bsSync.Read(bytePacketID);
	bsSync.Read(playerId);
	bsSync.Read(icSync.VehicleID);

	// keys
	bsSync.Read(icSync.lrAnalog);
	bsSync.Read(icSync.udAnalog);
	bsSync.Read(icSync.wKeys);

	// quaternion
	bsSync.ReadNormQuat(icSync.quat.w, icSync.quat.x, icSync.quat.y, icSync.quat.z);

	// position
	bsSync.Read((char*)&icSync.vecPos, sizeof(VECTOR));

	// speed
	bsSync.ReadVector(icSync.vecMoveSpeed.X, icSync.vecMoveSpeed.Y, icSync.vecMoveSpeed.Z);

	// vehicle health
	uint16_t wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	icSync.fCarHealth = (float)wTempVehicleHealth;

	// health/armour
	uint8_t byteHealthArmour;
	uint8_t byteArmTemp=0, byteHlTemp=0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) icSync.bytePlayerArmour = 100;
	else if(byteArmTemp == 0) icSync.bytePlayerArmour = 0;
	else icSync.bytePlayerArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) icSync.bytePlayerHealth = 100;
	else if(byteHlTemp == 0) icSync.bytePlayerHealth = 0;
	else icSync.bytePlayerHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	uint8_t byteTempWeapon;
	bsSync.Read(byteTempWeapon);
	icSync.byteCurrentWeapon ^= (byteTempWeapon ^ icSync.byteCurrentWeapon) & 0x3F;

	bool bCheck;

	// siren
	bsSync.Read(bCheck);
	if(bCheck) icSync.byteSirenOn = 1;
	// landinggear
	bsSync.Read(bCheck);
	if(bCheck) icSync.byteLandingGearState = 1;
	// train speed
	bsSync.Read(bCheck);
	if(bCheck) bsSync.Read(icSync.fTrainSpeed);
	// triler id
	bsSync.Read(bCheck);
	if(bCheck) bsSync.Read(icSync.TrailerID);

	if(m_pPlayerPool)
	{
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
			pPlayer->StoreInCarFullSyncData(&icSync, 0);
	}
}

void CNetGame::Packet_PassengerSync(Packet* pkt)
{
	CRemotePlayer *pPlayer;
	uint8_t bytePacketID;
	PLAYERID playerId;
	PASSENGER_SYNC_DATA psSync;

	if(GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsPassengerSync((unsigned char *)pkt->data, pkt->length, false);
	bsPassengerSync.Read(bytePacketID);
	bsPassengerSync.Read(playerId);
	bsPassengerSync.Read((char*)&psSync, sizeof(PASSENGER_SYNC_DATA));

	if(m_pPlayerPool)
	{
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if(pPlayer)
			pPlayer->StorePassengerFullSyncData(&psSync);
	}
}

void CNetGame::Packet_MarkersSync(Packet *pkt)
{
	CRemotePlayer *pPlayer;
	int			iNumberOfPlayers = 0;
	PLAYERID	playerId;
	short		sPos[3];
	bool		bIsPlayerActive;
	uint8_t 	unk0 = 0;

	if(GetGameState() != GAMESTATE_CONNECTED)
		return;

	RakNet::BitStream bsMarkersSync((unsigned char *)pkt->data, pkt->length, false);
	bsMarkersSync.Read(unk0);
	bsMarkersSync.Read(iNumberOfPlayers);

	if(iNumberOfPlayers)
	{
		for(int i=0; i<iNumberOfPlayers; i++)
		{
			bsMarkersSync.Read(playerId);
			bsMarkersSync.ReadCompressed(bIsPlayerActive);

			if(bIsPlayerActive)
			{
				bsMarkersSync.Read(sPos[0]);
				bsMarkersSync.Read(sPos[1]);
				bsMarkersSync.Read(sPos[2]);
			}

			if(playerId < MAX_PLAYERS && m_pPlayerPool->GetSlotState(playerId))
			{
				pPlayer = m_pPlayerPool->GetAt(playerId);
				if(pPlayer)
				{
					if(bIsPlayerActive)
						pPlayer->ShowGlobalMarker(sPos[0], sPos[1], sPos[2]);
					else pPlayer->HideGlobalMarker();
				}
			}
		}
	}
}

void CNetGame::Packet_BulletSync(Packet *pkt)
{  
	uint8_t bytePacketID;
	PLAYERID PlayerId;
	BULLET_SYNC_DATA btSync;
	RakNet::BitStream bsBulletSync((unsigned char*)pkt->data, pkt->length, false);

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsBulletSync.Read(bytePacketID);
	bsBulletSync.Read(PlayerId);
	bsBulletSync.Read((char*)&btSync, sizeof(BULLET_SYNC_DATA));

	CRemotePlayer *pPlayer = m_pPlayerPool->GetAt(PlayerId);
	if(pPlayer)
		pPlayer->StoreBulletFullSyncData(&btSync);
}

void CNetGame::Packet_AimSync(Packet * p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsAimSync((unsigned char*)p->data, p->length, false);
	AIM_SYNC_DATA aimSync;
	uint8_t bytePacketID = 0;
	uint16_t bytePlayerID = 0;

	if (GetGameState() != GAMESTATE_CONNECTED)
		return;

	bsAimSync.Read(bytePacketID);
	bsAimSync.Read(bytePlayerID);
	bsAimSync.Read((char*)&aimSync, sizeof(AIM_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if (pPlayer)
		pPlayer->UpdateAimFromSyncData(&aimSync);
}
#define RPC_TWITTER_MAIN				0x33

#define RPC_IPHONE						0x34
#define RPC_SHOWWELCOME 				0x35

#define RPC_DONATE	0x37
#define RPC_DONATE_UPD	0x38
#define RPC_DONATE_SHOWSC	0x39
#define RPC_DONATE_BUYCAR	0x41
#define RPC_BR_NOTIFICATION 0x3A


#include "../util/CJavaWrapper.h"

void CNetGame::Packet_CustomRPC(Packet* p)
{
	
	RakNet::BitStream bs((unsigned char*)p->data, p->length, false);
	uint8_t packetID;
	uint32_t rpcID;

	bs.Read(packetID);
	bs.Read(rpcID);


	pChatWindow->AddDebugMessage("p %d rpc %d", packetID, rpcID);
	switch (rpcID)
	{
		case RPC_SHOWWELCOME:
		{
			uint8_t isReg = 1;
			bs.Read(isReg);
			bool a = isReg == 1 ? true : false;
			g_pJavaWrapper->ShowWelcome(a);
			return;
		}
		case RPC_TWITTER_MAIN : 
		{
			char szBuff[4096+1];
			char playername[64*54];
			char msg[64*54];
			char url[64*54];

			//Message
			uint16_t lenMessage;
			bs.Read(lenMessage);

			memset(msg, 0, sizeof(msg));
			memset(szBuff, 0, sizeof(szBuff));

			bs.Read(szBuff, lenMessage);
			cp1251_to_utf8(msg, szBuff);

			//delay
			int duration;
			bs.Read(duration);

			//Url image
			uint16_t lenUrl;
			bs.Read(lenUrl);

			memset(url, 0, sizeof(url));
			memset(szBuff, 0, sizeof(szBuff));

			bs.Read(szBuff, lenUrl);
			cp1251_to_utf8(url, szBuff);

			//Message
			uint16_t lenName;
			bs.Read(lenName);

			memset(playername, 0, sizeof(playername));
			memset(szBuff, 0, sizeof(szBuff));

			bs.Read(szBuff, lenName);
			cp1251_to_utf8(playername, szBuff);

			g_pJavaWrapper->PostTwitter(msg, duration, url, playername);
			g_pJavaWrapper->ShowNotification(duration, playername, msg, url);
			return;
		}

		case RPC_IPHONE :
		{
			int status;
			bs.Read(status);
			if(status == 1){
				g_pJavaWrapper->ShowPhone();
			} else{
				g_pJavaWrapper->HidePhone();
			}
		}	
		case RPC_DONATE:
		{
			int money;
			bs.Read(money);

			int bc;
			bs.Read(bc);
		    
			g_pJavaWrapper->ShowDonate(money, bc);
			return;
		}
		case RPC_DONATE_UPD:
		{
			int money;
			bs.Read(money);

			int bc;
			bs.Read(bc);
		    
			g_pJavaWrapper->UpdateDonate(money, bc);
			return;
		}
		case RPC_BR_NOTIFICATION:
    {
        char szBuff[2048];
        uint16_t lenText;
        int type;
        int duration;

        // Đọc dữ liệu từ BitStream
        bs.Read(lenText);
        
        // Đảm bảo không vượt quá kích thước buffer
        if (lenText >= sizeof(szBuff)) lenText = sizeof(szBuff) - 1;
        
        bs.Read(szBuff, lenText);
        szBuff[lenText] = '\0'; // Thêm ký tự kết thúc chuỗi để đảm bảo an toàn
        
        bs.Read(type);
        bs.Read(duration);
        if(g_pJavaWrapper) {
            g_pJavaWrapper->ShowBrNotification(szBuff, type, duration);
        }
        return;
    }
		case RPC_DONATE_SHOWSC:
		{	
			int money;
			bs.Read(money);

			int bc;
			bs.Read(bc);

			g_pJavaWrapper->show_sc(money, bc);
			return;
		}
	}
}
