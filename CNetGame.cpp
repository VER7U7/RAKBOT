#include "CNetGame.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<uint32_t> CNetGame::allThisID;

void CNetGame::MainProcess() {
	std::chrono::milliseconds interval(1000 / 1000);
	while (MainProcessActive) {
		std::this_thread::sleep_for(interval);
		ticksCount++;
		Process();
	}
}

void CNetGame::InitVars() {
	//init player vars
	fNormalModePos[0] = 0;
	fNormalModePos[1] = 0;
	fNormalModePos[2] = 0;
	resetPools(1);
	uiAnimation = 0;
	vecMoveSpeed[0] = 0;
	vecMoveSpeed[1] = 0;
	vecMoveSpeed[2] = 0;
	fQuaternion[0] = 0;
	fQuaternion[1] = 0;
	fQuaternion[2] = 0;
	fQuaternion[3] = 0;
	vecSurfOffsets[0] = 0;
	vecSurfOffsets[1] = 0;
	vecSurfOffsets[2] = 0;
	usSurfInfo = 0;
	byteSpecialAction = 0;
	lrAnalog = 0;
	udAnalog = 0;
	usKeys = 0;

	//checkpoint init
	fCurrentCheckpoint[0] = 0;
	fCurrentCheckpoint[1] = 0;
	fCurrentCheckpoint[2] = 0;

	bStart = false;

	//for (int i = 0; i < 100; i++)
		//m_dwMapIcons[i] = 0;
}

CNetGame::CNetGame(const char* szHostOrIp, int iPort, const char* szPlayerName, const char* szPass, int AUTH_TYPE) {
	srand(time(0));
	client = RakNetworkFactory::GetRakClientInterface();
	client->SetMTUSize(576);
	client->SetPassword("");

	lastConnectTime = GetTicksCount();

	CustomProcess = nullptr;
	StartClient();
	std::thread MainProc(&CNetGame::MainProcess, this);
	MainProc.detach();

	ThisID = GetNewThisID();
	rpcFactory = new CNetRPC(this, ThisID);
	rpcFactory->RegisterRPC(client);

	//str copy
	strcpy(m_szHostName, "HOST NAME");
	strncpy(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	strcpy(m_szPlayerName, szPlayerName);
	strcpy(m_szClientVersion, "0.3.7");
	m_iPort = iPort;

	GameState = GAMESTATE_WAIT_CONNECT;

	InitVars();

	m_iAuthType = AUTH_TYPE;
	switch (AUTH_TYPE) {
	default:
		strcpy(AUTH_BS, "E02262CF28BC542486C558D4BE9EFB716592AFAF8B");
		break;
	}
}

CNetGame::~CNetGame() {
	client->Disconnect(0);
	rpcFactory->UnRegisterRPC(client);
	RakNetworkFactory::DestroyRakClientInterface(client);
	RemoveThisID();
	this->StopClient();
}

unsigned char GetPacketID(Packet* p)
{
	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		if (p->length > sizeof(unsigned char) + sizeof(unsigned int))
			return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned int)];
		else
			return 0;
	}
	else
		return (unsigned char)p->data[0];
}

void CNetGame::Process() {

	if (Ptime == 0)
		Ptime = GetTicksCount();
	bool Process = false;
	if (GetTicksCount() - Ptime >= 1000 / 30) {
		UpdateNetwork();
		Ptime = GetTicksCount();
		Process = true;
	}

	////TODO: PlayerPool and HoldTime

	if (GetGameState() == GAMESTATE_CONNECTED) {
		//if connected
		
		if (GetTickCount() - uiLastInfoUpdate >= 1000) {
			uiLastInfoUpdate = GetTickCount();

		}

		if (CustomProcess != nullptr) {
			if (GetTicksCount() - uiLastCustomProcessTick >= 1000 / 30) {
				CustomProcess(this);
				uiLastCustomProcessTick = GetTicksCount();
			}
		}

		//update stats
		if ((GetTickCount() - uiLastStatsUpdate >= 1000) || iMoney != iLastMoney || iDrunkLevel != iLastDrunkLevel) {
			RakNet::BitStream bsSend;

			bsSend.Write((BYTE)ID_STATS_UPDATE);

			iDrunkLevel -= (rand() % 90 + 20);

			if (iDrunkLevel < 0)
				iDrunkLevel = 0;

			bsSend.Write(iMoney);
			bsSend.Write(iDrunkLevel);

			client->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);

			iLastMoney = iMoney;
			iLastDrunkLevel = iDrunkLevel;

			uiLastStatsUpdate = GetTickCount();
		}

		//std::cout << GetTickCount() << " " << dwLastOnFootDataSentTick << std::endl;

		if (!iSpawned) {
			if (bSpawnRequest) {
				iSpawned = true;
			}
			if (sendRequestClass) {
				sampRequestClass(0, this);
			}
			sampSpawn(this);
		}
		else {
			if (!bIsSpectating) {
				if (!inVehicle) 
					onFootUpdateAtNormalPos(this);
				else {
					if (isPassenger)
						SendPassengerFullSyncData(inVehicleID, this);
					else
						SendInCarFullSyncData(this->incarData, inVehicleID, this);
				}
			}
			else {
				spectatorUpdate(this);
				iSpawned = false;
			}
		}


	}
	else {
		//else not connected
		if (CustomProcessNC != nullptr) {
			if (GetTicksCount() - uiLastCustomProcessTick >= 1000 / 30) {
				CustomProcessNC(this);
				uiLastCustomProcessTick = GetTicksCount();
			}
		}
	}

	if (GetGameState() == GAMESTATE_WAIT_CONNECT &&
		(GetTicksCount() - lastConnectTime) > 3000) {

		std::cout << "Connect to server..." << std::endl;
		client->Connect(m_szHostOrIp, m_iPort, 0, 0, 5);
		lastConnectTime = GetTicksCount();
		SetGameState(GAMESTATE_CONNECTING);
	}

}

void CNetGame::UpdateNetwork() {

	Packet* pkt = nullptr;
	unsigned char packetIditenfier;

	while (pkt = client->Receive()) {
		//Log("Receive");
		packetIditenfier = GetPacketID(pkt);

		//std::cout << (int)packetIditenfier << std::endl;
		switch (packetIditenfier) {
			case ID_AUTH_KEY:
				Log("Incoming packet: IP_AUTH_KEY");
				Packet_AuthKey(pkt);
				break;

			case ID_CONNECTION_ATTEMPT_FAILED:
				Log("Connection is fail");
				SetGameState(GAMESTATE_WAIT_CONNECT);
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				SetGameState(GAMESTATE_WAIT_CONNECT);
				Log("Server is full");
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				Packet_DisconnectionNotification(pkt);
				break;

			case ID_CONNECTION_LOST:
				Packet_ConnectionLost(pkt);
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				Packet_ConnectionSucceeded(pkt);
				Log("Connected. Joining the game...");
				break;

			case ID_CONNECTION_BANNED:
				Log("Connection banned from this server");
				SetGameState(GAMESTATE_WAIT_CONNECT);
				break;

			case ID_INVALID_PASSWORD:
				Log("No valid password");
				client->Disconnect(0);
				break;

			case ID_PLAYER_SYNC:
				//Log("Incoming packet: ID_PLAYER_SYNC");
				Packet_PlayerSync(pkt);
				break;

			case ID_VEHICLE_SYNC:
				//Log("Incoming packet: ID_VEHICLE_SYNC");
				Packet_VehicleSync(pkt);
				break;

			case ID_PASSENGER_SYNC:
				//Log("Incoming packet: ID_PASSENGER_SYNC");
				Packet_PassengerSync(pkt);
				break;

			case ID_MARKERS_SYNC:
				//Log("Incoming packet: ID_MARKERS_SYNC");
				Packet_MarkersSync(pkt);
				break;

			case ID_AIM_SYNC:
				//Log("Incoming packet: ID_AIM_SYNC");
				Packet_AimSync(pkt);
				break;

			case ID_BULLET_SYNC:
				Log("Incoming packet: ID_BULLET_SYNC");
				Packet_BulletSync(pkt);
				break;

			case ID_TRAILER_SYNC:
				Log("Incoming packet: ID_TRAILER_SYNC");
				Packet_TrailerSync(pkt);
				break;

			//case ID_CUSTOM_RPC:
			//	Log("Incoming packet: ID_CUSTOM_RPC");
			//	//Packet_CustomRPC(pkt);
			//	break;
		}

		client->DeallocatePacket(pkt);
	}
	UpdatePlayerScoresAndPings(1, 3000, client);
}

int gen_gpci(char buf[64], unsigned long factor) /* by bartekdvd */
{
	unsigned char out[6 * 4] = { 0 };

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 6 * 4; ++i)
		out[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	out[(6 * 4)-1] = 0;

	BIG_NUM_MUL((unsigned long*)out, (unsigned long*)out, factor);

	unsigned int notzero = 0;
	buf[0] = '0'; buf[1] = '\0';

	if (factor == 0) return 1;

	int pos = 0;
	for (int i = 0; i < 24; i++)
	{
		unsigned char tmp = out[i] >> 4;
		unsigned char tmp2 = out[i] & 0x0F;

		if (notzero || tmp)
		{
			buf[pos++] = (char)((tmp > 9) ? (tmp + 55) : (tmp + 48));
			if (!notzero) notzero = 1;
		}

		if (notzero || tmp2)
		{
			buf[pos++] = (char)((tmp2 > 9) ? (tmp2 + 55) : (tmp2 + 48));
			if (!notzero) notzero = 1;
		}
	}
	buf[pos] = 0;

	return pos;
}

void gen_auth_key(char buf[260], char* auth_in) {}
void CNetGame::Packet_AuthKey(Packet* p) {
	RakNet::BitStream bsAuth((unsigned char*)p->data, p->length, false);

	if (m_iAuthType == AUTH_BLACKRUSSIA) {
		uint8_t byteAuthLen;
		char szAuth[260];

		bsAuth.IgnoreBits(8);
		bsAuth.Read(byteAuthLen);
		bsAuth.Read(szAuth, byteAuthLen);
		szAuth[byteAuthLen] = '\0';

		char szAuthKey[260];
		gen_auth_key(szAuthKey, szAuth);

		RakNet::BitStream bsKey;
		uint8_t byteAuthKeyLen = (uint8_t)strlen(szAuthKey);

		bsKey.Write((uint8_t)ID_AUTH_KEY);
		bsKey.Write((uint8_t)byteAuthKeyLen);
		bsKey.Write(szAuthKey, byteAuthKeyLen);
		client->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);
	}else {
		char* auth_key;
		bool found_key = false;

		for (int x = 0; x < 512; x++)
		{
			if (!strcmp(((char*)p->data + 2), AuthKeyTable[x][0]))
			{
				auth_key = AuthKeyTable[x][1];
				found_key = true;
			}
		}

		if (found_key)
		{
			RakNet::BitStream bsKey;
			BYTE byteAuthKeyLen;

			byteAuthKeyLen = (BYTE)strlen(auth_key);

			bsKey.Write((BYTE)ID_AUTH_KEY);
			bsKey.Write((BYTE)byteAuthKeyLen);
			bsKey.Write(auth_key, byteAuthKeyLen);

			client->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, NULL);
		}
	}
}

void CNetGame::Packet_ConnectionSucceeded(Packet* p) {
	SetGameState(GAMESTATE_AWAIT_JOIN);

	RakNet::BitStream bsSuccAuth((unsigned char*)p->data, p->length, false);
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
	{
		sum += std::atoi(s.c_str());
	}

	int iVersion = 4057;//NETGAME_VERSION;
	char byteMod = 0x01;
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;

	char byteAuthBSLen = (char)strlen(AUTH_BS);
	char byteNameLen = (char)strlen(m_szPlayerName);
	char byteClientverLen = (char)strlen(m_szClientVersion);

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(m_szPlayerName, byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write(AUTH_BS, byteAuthBSLen);
	bsSend.Write(byteClientverLen);
	bsSend.Write(m_szClientVersion, byteClientverLen);

	//CClientInfo::WriteClientInfoToBitStream(bsSend);

	client->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::Packet_PlayerSync(Packet* p)
{
	RakNet::BitStream bsPlayerSync((unsigned char*)p->data, p->length, false);

	//Log("Packet_PlayerSync: %d \n%s\n", p->length, DumpMem((unsigned char *)p->data, p->length));
	//Log(__FUNCTION__);

	bool bHasLR, bHasUD;
	bool bHasSurfInfo;

	unsigned short playerId = 0;

	bsPlayerSync.IgnoreBits(8);
	bsPlayerSync.Read(playerId);
	bStart = true;

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	playersInfo[playerId].incarData.VehicleID = -1;

	//std::cout << playerId << std::endl;
	// clear last data
	memset(&playersInfo[playerId].onfootData, 0, sizeof(ONFOOT_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if (bHasLR) bsPlayerSync.Read(playersInfo[playerId].onfootData.lrAnalog);

	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if (bHasUD) bsPlayerSync.Read(playersInfo[playerId].onfootData.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(playersInfo[playerId].onfootData.wKeys);

	//std::cout << "LR UD KEY: " << (int)playersInfo[playerId].onfootData.lrAnalog << " "
	//	<< (int)playersInfo[playerId].onfootData.udAnalog << " "
	//	<< (int)playersInfo[playerId].onfootData.wKeys << std::endl;

	// VECTOR POS
	bsPlayerSync.Read(playersInfo[playerId].onfootData.vecPos[0]);
	bsPlayerSync.Read(playersInfo[playerId].onfootData.vecPos[1]);
	bsPlayerSync.Read(playersInfo[playerId].onfootData.vecPos[2]);

	/*std::cout << "POS x:" << playersInfo[playerId].onfootData.vecPos[0] << " y:"
		<< playersInfo[playerId].onfootData.vecPos[1] << " z:"
		<< playersInfo[playerId].onfootData.vecPos[2] << std::endl;*/

	// ROTATION
	bsPlayerSync.ReadNormQuat(
		playersInfo[playerId].onfootData.fQuaternion[0],
		playersInfo[playerId].onfootData.fQuaternion[1],
		playersInfo[playerId].onfootData.fQuaternion[2],
		playersInfo[playerId].onfootData.fQuaternion[3]);

	/*std::cout << "ROT x:" << playersInfo[playerId].onfootData.fQuaternion[0]
		<< " y:" << playersInfo[playerId].onfootData.fQuaternion[1]
		<< " z:" << playersInfo[playerId].onfootData.fQuaternion[2]
		<< " w:" << playersInfo[playerId].onfootData.fQuaternion[3] << std::endl;*/


	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteHealth, byteArmour;
	BYTE byteArmTemp = 0, byteHlTemp = 0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if (byteArmTemp == 0xF) byteArmour = 100;
	else if (byteArmTemp == 0) byteArmour = 0;
	else byteArmour = byteArmTemp * 7;

	if (byteHlTemp == 0xF) byteHealth = 100;
	else if (byteHlTemp == 0) byteHealth = 0;
	else byteHealth = byteHlTemp * 7;

	playersInfo[playerId].onfootData.byteHealth = byteHealth;
	playersInfo[playerId].onfootData.byteArmour = byteArmour;


	// CURRENT WEAPON
	bsPlayerSync.Read(playersInfo[playerId].onfootData.byteCurrentWeapon);

	// Special Action
	bsPlayerSync.Read(playersInfo[playerId].onfootData.byteSpecialAction);

	// READ MOVESPEED VECTORS
	bsPlayerSync.ReadVector(
		playersInfo[playerId].onfootData.vecMoveSpeed[0],
		playersInfo[playerId].onfootData.vecMoveSpeed[1],
		playersInfo[playerId].onfootData.vecMoveSpeed[2]);


	bsPlayerSync.Read(bHasSurfInfo);
	if (bHasSurfInfo)
	{
		bsPlayerSync.Read(playersInfo[playerId].onfootData.wSurfInfo);
		bsPlayerSync.ReadVector(playersInfo[playerId].onfootData.vecSurfOffsets[0],
			playersInfo[playerId].onfootData.vecSurfOffsets[1],
			playersInfo[playerId].onfootData.vecSurfOffsets[2]);
	}
	else
		playersInfo[playerId].onfootData.wSurfInfo = INVALID_VEHICLE_ID;


	bool bHasAnimInfo;
	bsPlayerSync.Read(bHasAnimInfo);
	if (bHasAnimInfo)
	{
		bsPlayerSync.Read(playersInfo[playerId].onfootData.uiAnimation);
	}
	else
	{
		playersInfo[playerId].onfootData.uiAnimation = 0b10000000000000000000000000000000;
	}

	
	uint32_t animBlend, animFlags;
	uint16_t animID = DecodeAnimation(playersInfo[playerId].onfootData.uiAnimation, &animBlend, &animFlags);

	//std::cout << animID << " " << animBlend << " " << animFlags << std::endl;


	if ((int)playerId == (int)this->playerID)
	{
		if (iPassengerNotificationSent)
		{
			SendExitVehicleNotification(playersInfo[playerId].incarData.VehicleID, this);
			iPassengerNotificationSent = 0;
		}

		iFollowingPassenger = 0;

		if (iDriverNotificationSent)
		{
			SendExitVehicleNotification(playersInfo[playerId].incarData.VehicleID, this);
			iDriverNotificationSent = 0;
		}

		iFollowingDriver = 0;
	}
}

void CNetGame::Packet_VehicleSync(Packet* p)
{
	RakNet::BitStream bsSync((unsigned char*)p->data, p->length, false);
	PLAYERID playerId;

	VEHICLEID VehicleID;
	bool bLandingGear;
	bool bHydra, bTrain, bTrailer;
	bool bSiren;

	//Log("Packet_VehicleSync: %d \n%s\n", p->length, DumpMem((unsigned char *)p->data, p->length));

	bsSync.IgnoreBits(8);
	bsSync.Read(playerId);
	bsSync.Read(VehicleID);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;
	if (VehicleID < 0 || VehicleID >= MAX_VEHICLES) return;

	// Follower passenger enter
	playersInfo[playerId].incarData.VehicleID = VehicleID;
	
	// clear last data
	memset(&playersInfo[playerId].incarData, 0, sizeof(INCAR_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsSync.Read(playersInfo[playerId].incarData.lrAnalog);

	// UP/DOWN KEYS
	bsSync.Read(playersInfo[playerId].incarData.udAnalog);

	// GENERAL KEYS
	bsSync.Read(playersInfo[playerId].incarData.wKeys);

	// ROLL / DIRECTION
	// ROTATION
	bsSync.ReadNormQuat(
		playersInfo[playerId].incarData.fQuaternion[0],
		playersInfo[playerId].incarData.fQuaternion[1],
		playersInfo[playerId].incarData.fQuaternion[2],
		playersInfo[playerId].incarData.fQuaternion[3]);

	// POSITION
	bsSync.Read(playersInfo[playerId].incarData.vecPos[0]);
	bsSync.Read(playersInfo[playerId].incarData.vecPos[1]);
	bsSync.Read(playersInfo[playerId].incarData.vecPos[2]);

	// SPEED
	bsSync.ReadVector(
		playersInfo[playerId].incarData.vecMoveSpeed[0],
		playersInfo[playerId].incarData.vecMoveSpeed[1],
		playersInfo[playerId].incarData.vecMoveSpeed[2]);

	// VEHICLE HEALTH
	WORD wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	playersInfo[playerId].incarData.fCarHealth = (float)wTempVehicleHealth;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE bytePlayerHealth, bytePlayerArmour;
	BYTE byteArmTemp = 0, byteHlTemp = 0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if (byteArmTemp == 0xF) bytePlayerArmour = 100;
	else if (byteArmTemp == 0) bytePlayerArmour = 0;
	else bytePlayerArmour = byteArmTemp * 7;

	if (byteHlTemp == 0xF) bytePlayerHealth = 100;
	else if (byteHlTemp == 0) bytePlayerHealth = 0;
	else bytePlayerHealth = byteHlTemp * 7;

	playersInfo[playerId].incarData.bytePlayerHealth = bytePlayerHealth;
	playersInfo[playerId].incarData.bytePlayerArmour = bytePlayerArmour;

	// CURRENT WEAPON
	bsSync.Read(playersInfo[playerId].incarData.byteCurrentWeapon);

	// SIREN
	bsSync.ReadCompressed(bSiren);
	if (bSiren)
		playersInfo[playerId].incarData.byteSirenOn = 1;

	// LANDING GEAR
	bsSync.ReadCompressed(bLandingGear);
	if (bLandingGear)
		playersInfo[playerId].incarData.byteLandingGearState = 1;

	// HYDRA THRUST ANGLE AND TRAILER ID
	bsSync.ReadCompressed(bHydra);
	bsSync.ReadCompressed(bTrailer);

	DWORD dwTrailerID_or_ThrustAngle;
	bsSync.Read(dwTrailerID_or_ThrustAngle);
	playersInfo[playerId].incarData.TrailerID_or_ThrustAngle = (WORD)dwTrailerID_or_ThrustAngle;

	// TRAIN SPECIAL
	WORD wSpeed;
	bsSync.ReadCompressed(bTrain);
	if (bTrain)
	{
		bsSync.Read(wSpeed);
		playersInfo[playerId].incarData.fTrainSpeed = (float)wSpeed;
	}
}

void CNetGame::Packet_PassengerSync(Packet* p)
{
	Log(__func__);
	RakNet::BitStream bsPassengerSync((unsigned char*)p->data, p->length, false);
	PLAYERID	playerId;
	PASSENGER_SYNC_DATA psSync;

	bsPassengerSync.IgnoreBits(8);
	bsPassengerSync.Read(playerId);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	bsPassengerSync.Read((PCHAR)&psSync, sizeof(PASSENGER_SYNC_DATA));

	// Followed wants to drive the vehicle
	playersInfo[playerId].passengerData.VehicleID = psSync.VehicleID;
}

void CNetGame::Packet_MarkersSync(Packet* p)
{
	//Log(__FUNCTION__);
	RakNet::BitStream bsMarkersSync((unsigned char*)p->data, p->length, false);

	int i, iNumberOfPlayers;
	PLAYERID playerID;
	short sPosX, sPosY, sPosZ;
	bool bIsPlayerActive;

	bsMarkersSync.IgnoreBits(8);
	bsMarkersSync.Read(iNumberOfPlayers);

	if (iNumberOfPlayers < 0 || iNumberOfPlayers > MAX_PLAYERS) return;

	for (i = 0; i < iNumberOfPlayers; i++)
	{

		bsMarkersSync.Read(playerID);

		if (playerID < 0 || playerID >= MAX_PLAYERS) return;

		bsMarkersSync.ReadCompressed(bIsPlayerActive);
		if (bIsPlayerActive == 0)
		{
			playersInfo[playerID].iGotMarkersPos = 0;
			continue;
		}

		bsMarkersSync.Read(sPosX);
		bsMarkersSync.Read(sPosY);
		bsMarkersSync.Read(sPosZ);

		playersInfo[playerID].iGotMarkersPos = 1;
		playersInfo[playerID].onfootData.vecPos[0] = (float)sPosX;
		playersInfo[playerID].onfootData.vecPos[1] = (float)sPosY;
		playersInfo[playerID].onfootData.vecPos[2] = (float)sPosZ;

		//Log("Packet_MarkersSync: %d %d %0.2f, %0.2f, %0.2f", playerID, bIsPlayerActive, (float)sPosX, (float)sPosY, (float)sPosZ);
	}
}

void CNetGame::Packet_AimSync(Packet* p)
{
	RakNet::BitStream bsAimSync((unsigned char*)p->data, p->length, false);
	PLAYERID playerId;

	//Log("Packet_AimSync:\n%s\n", DumpMem((unsigned char *)p->data, p->length));

	bsAimSync.IgnoreBits(8);
	bsAimSync.Read(playerId);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	memset(&playersInfo[playerId].aimData, 0, sizeof(AIM_SYNC_DATA));

	bsAimSync.Read((PCHAR)&playersInfo[playerId].aimData, sizeof(AIM_SYNC_DATA));
}

void CNetGame::Packet_BulletSync(Packet* pkt)
{
	RakNet::BitStream bsBulletSync((unsigned char*)pkt->data, pkt->length, false);

	if (m_iLagCompensation)
	{
		PLAYERID PlayerID;

		bsBulletSync.IgnoreBits(8);
		bsBulletSync.Read(PlayerID);

		if (PlayerID < 0 || PlayerID >= MAX_PLAYERS) return;

		memset(&playersInfo[PlayerID].bulletData, 0, sizeof(BULLET_SYNC_DATA));

		bsBulletSync.Read((PCHAR)&playersInfo[PlayerID].bulletData, sizeof(BULLET_SYNC_DATA));

		std::cout << (int)playersInfo[PlayerID].bulletData.bWeaponID << std::endl;

		PLAYERID copyingID = this->playerID;

		if (copyingID != (PLAYERID)-1)
		{
			if (copyingID == PlayerID)
				SendBulletData(&playersInfo[PlayerID].bulletData, this);
		}
	}
}

void CNetGame::Packet_TrailerSync(Packet* p)
{
	RakNet::BitStream bsSpectatorSync((unsigned char*)p->data, p->length, false);

	PLAYERID playerId;
	//TRAILER_SYNC_DATA trSync;

	bsSpectatorSync.IgnoreBits(8);
	bsSpectatorSync.Read(playerId);
	//bsSpectatorSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));
}

void CNetGame::Packet_CustomRPC(Packet* p)
{
}

void CNetGame::Packet_DisconnectionNotification(Packet* p){
	client->Disconnect(2000);
	Log("Server closed this connection.");
}

void CNetGame::Packet_ConnectionLost(Packet* p) {
	if (client)
		client->Disconnect(0);
	Log("Connection lost. Reconnecting...");
	resetPools(1);
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

void CNetGame::resetPools(int iRestart)
{
	memset(playersInfo, 0, sizeof(stPlayerInfo)*MAX_PLAYERS);
	memset(vehiclesPool, 0, sizeof(stVehiclePool)*MAX_PLAYERS);

	if (iRestart)
	{
		SetGameState(GAMESTATE_WAIT_CONNECT);
		iMoney = 0;
		iDrunkLevel = 0;
		iLocalPlayerSkin = 0;

		fPlayerHealth = 100.0f;
		fPlayerArmour = 0.0f;

		//Sleep(dwTimeReconnect);
	}
}

void CNetGame::UpdatePlayerScoresAndPings(int iWait, int iMS, RakClientInterface* pRakClient)
{
	static DWORD dwLastUpdateTick = 0;

	if (iWait)
	{
		if ((GetTickCount() - dwLastUpdateTick) > (DWORD)iMS)
		{
			dwLastUpdateTick = GetTickCount();
			RakNet::BitStream bsParams;
			pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
		}
	}
	else
	{
		RakNet::BitStream bsParams;
		pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	}
}

uint32_t GetAnimationIndexFlag(int AnimID, uint32_t flags, uint32_t blend)
{
	uint32_t dwAnim = 0;

	int iAnimIdx = AnimID;

	uint32_t hardcodedBlend = blend;
	hardcodedBlend <<= 16;

	uint32_t hardcodedFlags = 0;

	if (iAnimIdx != 0)
	{
		hardcodedFlags = flags;
	}
	else
	{
		hardcodedFlags = 0b10000000;
		iAnimIdx = 1189;
	}

	hardcodedFlags <<= 24;

	uint16_t usAnimidx = (uint16_t)iAnimIdx;

	dwAnim = (uint32_t)usAnimidx;
	dwAnim |= hardcodedBlend;
	dwAnim |= hardcodedFlags;

	return dwAnim;
}

uint32_t GetAnimationIndexFlag(int AnimID)
{
	uint32_t dwAnim = 0;

	int iAnimIdx = AnimID;

	uint32_t hardcodedBlend = 4;
	hardcodedBlend <<= 16;

	uint32_t hardcodedFlags = 0;

	if (iAnimIdx != 0)
	{
		hardcodedFlags = 19;
	}
	else
	{
		hardcodedFlags = 0b10000000;
		iAnimIdx = 1189;
	}

	hardcodedFlags <<= 24;

	uint16_t usAnimidx = (uint16_t)iAnimIdx;

	dwAnim = (uint32_t)usAnimidx;
	dwAnim |= hardcodedBlend;
	dwAnim |= hardcodedFlags;

	return dwAnim;
}

uint16_t DecodeAnimation(uint32_t encAnim, uint32_t* outBlend, uint32_t* outFlags) {
	*outBlend = (encAnim >> 16) & 0xFF;
	*outFlags = (encAnim >> 24) & 0xFF;
	return (uint16_t)(encAnim & 0xFFFF); //animID
}
uint16_t DecodeAnimation(uint32_t encAnim) {
	return (uint16_t)(encAnim & 0xFFFF); //animID
}

bool CNetGame::SetCustomRPC(int customID, void (*CustomRPC)(CNetGame* netgame, RPCParameters* rpcParams)) {
	switch (customID) {
	case CUSTOM_ServerJoin:
		CustomServerJoin = CustomRPC;
		break;
	case CUSTOM_ServerQuit:
		CustomServerQuit = CustomRPC;
		break;
	case CUSTOM_InitGame:
		CustomInitGame = CustomRPC;
		break;
	case CUSTOM_WorldPlayerAdd:
		CustomWorldPlayerAdd = CustomRPC;
		break;
	case CUSTOM_WorldPlayerDeath:
		CustomWorldPlayerDeath = CustomRPC;
		break;
	case CUSTOM_WorldPlayerRemove:
		CustomWorldPlayerRemove = CustomRPC;
		break;
	case CUSTOM_WorldVehicleAdd:
		CustomWorldVehicleAdd = CustomRPC;
		break;
	case CUSTOM_WorldVehicleRemove:
		CustomWorldVehicleRemove = CustomRPC;
		break;
	case CUSTOM_ConnectionRejected:
		CustomConnectionRejected = CustomRPC;
		break;
	case CUSTOM_ClientMessage:
		CustomClientMessage = CustomRPC;
		break;
	case CUSTOM_Chat:
		CustomChat = CustomRPC;
		break;
	case CUSTOM_UpdateScoresPingsIPs:
		CustomUpdateScoresPingsIPs = CustomRPC;
		break;
	case CUSTOM_SetCheckpoint:
		CustomSetCheckpoint = CustomRPC;
		break;
	case CUSTOM_DisableCheckpoint:
		CustomDisableCheckpoint = CustomRPC;
		break;
	case CUSTOM_Pickup:
		CustomPickup = CustomRPC;
		break;
	case CUSTOM_DestroyPickup:
		CustomDestroyPickup = CustomRPC;
		break;
	case CUSTOM_RequestClass:
		CustomRequestClass = CustomRPC;
		break;
	case CUSTOM_ScrInitMenu:
		CustomScrInitMenu = CustomRPC;
		break;
	case CUSTOM_ScrDialogBox:
		CustomScrDialogBox = CustomRPC;
		break;
	case CUSTOM_ScrGameText:
		CustomScrGameText = CustomRPC;
		break;
	case CUSTOM_ScrPlayAudioStream:
		CustomScrPlayAudioStream = CustomRPC;
		break;
	case CUSTOM_ScrSetDrunkLevel:
		CustomScrSetDrunkLevel = CustomRPC;
		break;
	case CUSTOM_ScrHaveSomeMoney:
		CustomScrHaveSomeMoney = CustomRPC;
		break;
	case CUSTOM_ScrResetMoney:
		CustomScrResetMoney = CustomRPC;
		break;
	case CUSTOM_ScrSetPlayerPos:
		CustomScrSetPlayerPos = CustomRPC;
		break;
	case CUSTOM_ScrSetFacingAngle:
		CustomScrSetFacingAngle = CustomRPC;
		break;
	case CUSTOM_ScrSetSpawnInfo:
		CustomScrSetSpawnInfo = CustomRPC;
		break;
	case CUSTOM_ScrSetPlayerHealth:
		CustomScrSetPlayerHealth = CustomRPC;
		break;
	case CUSTOM_ScrSetPlayerArmour:
		CustomScrSetPlayerArmour = CustomRPC;
		break;
	case CUSTOM_ScrSetPlayerSkin:
		CustomScrSetPlayerSkin = CustomRPC;
		break;
	case CUSTOM_ScrCreateObject:
		CustomScrCreateObject = CustomRPC;
		break;
	case CUSTOM_ScrCreate3DTextLabel:
		CustomScrCreate3DTextLabel = CustomRPC;
		break;
	case CUSTOM_ScrShowTextDraw:
		CustomScrShowTextDraw = CustomRPC;
		break;
	case CUSTOM_ScrHideTextDraw:
		CustomScrHideTextDraw = CustomRPC;
		break;
	case CUSTOM_ScrEditTextDraw:
		CustomScrEditTextDraw = CustomRPC;
		break;
	case CUSTOM_ScrTogglePlayerSpectating:
		CustomScrTogglePlayerSpectating = CustomRPC;
		break;
	case CUSTOM_GiveTakeDamage:
		CustomGiveTakeDamage = CustomRPC;
		break;
	case CUSTOM_RequestSpawn:
		CustomRequestSpawn = CustomRPC;
		break;
	case CUSTOM_EnterVehicle:
		CustomEnterVehicle = CustomRPC;
		break;
	case CUSTOM_ExitVehicle:
		CustomExitVehicle = CustomRPC;
		break;

	default:
		return false;
	}
	return true;
}

bool CNetGame::DestroyCustomRPC(int customID) {
	switch (customID) {
	case CUSTOM_ServerJoin:
		CustomServerJoin = nullptr;
		break;
	case CUSTOM_ServerQuit:
		CustomServerQuit = nullptr;
		break;
	case CUSTOM_InitGame:
		CustomInitGame = nullptr;
		break;
	case CUSTOM_WorldPlayerAdd:
		CustomWorldPlayerAdd = nullptr;
		break;
	case CUSTOM_WorldPlayerDeath:
		CustomWorldPlayerDeath = nullptr;
		break;
	case CUSTOM_WorldPlayerRemove:
		CustomWorldPlayerRemove = nullptr;
		break;
	case CUSTOM_WorldVehicleAdd:
		CustomWorldVehicleAdd = nullptr;
		break;
	case CUSTOM_WorldVehicleRemove:
		CustomWorldVehicleRemove = nullptr;
		break;
	case CUSTOM_ConnectionRejected:
		CustomConnectionRejected = nullptr;
		break;
	case CUSTOM_ClientMessage:
		CustomClientMessage = nullptr;
		break;
	case CUSTOM_Chat:
		CustomChat = nullptr;
		break;
	case CUSTOM_UpdateScoresPingsIPs:
		CustomUpdateScoresPingsIPs = nullptr;
		break;
	case CUSTOM_SetCheckpoint:
		CustomSetCheckpoint = nullptr;
		break;
	case CUSTOM_DisableCheckpoint:
		CustomDisableCheckpoint = nullptr;
		break;
	case CUSTOM_Pickup:
		CustomPickup = nullptr;
		break;
	case CUSTOM_DestroyPickup:
		CustomDestroyPickup = nullptr;
		break;
	case CUSTOM_RequestClass:
		CustomRequestClass = nullptr;
		break;
	case CUSTOM_ScrInitMenu:
		CustomScrInitMenu = nullptr;
		break;
	case CUSTOM_ScrDialogBox:
		CustomScrDialogBox = nullptr;
		break;
	case CUSTOM_ScrGameText:
		CustomScrGameText = nullptr;
		break;
	case CUSTOM_ScrPlayAudioStream:
		CustomScrPlayAudioStream = nullptr;
		break;
	case CUSTOM_ScrSetDrunkLevel:
		CustomScrSetDrunkLevel = nullptr;
		break;
	case CUSTOM_ScrHaveSomeMoney:
		CustomScrHaveSomeMoney = nullptr;
		break;
	case CUSTOM_ScrResetMoney:
		CustomScrResetMoney = nullptr;
		break;
	case CUSTOM_ScrSetPlayerPos:
		CustomScrSetPlayerPos = nullptr;
		break;
	case CUSTOM_ScrSetFacingAngle:
		CustomScrSetFacingAngle = nullptr;
		break;
	case CUSTOM_ScrSetSpawnInfo:
		CustomScrSetSpawnInfo = nullptr;
		break;
	case CUSTOM_ScrSetPlayerHealth:
		CustomScrSetPlayerHealth = nullptr;
		break;
	case CUSTOM_ScrSetPlayerArmour:
		CustomScrSetPlayerArmour = nullptr;
		break;
	case CUSTOM_ScrSetPlayerSkin:
		CustomScrSetPlayerSkin = nullptr;
		break;
	case CUSTOM_ScrCreateObject:
		CustomScrCreateObject = nullptr;
		break;
	case CUSTOM_ScrCreate3DTextLabel:
		CustomScrCreate3DTextLabel = nullptr;
		break;
	case CUSTOM_ScrShowTextDraw:
		CustomScrShowTextDraw = nullptr;
		break;
	case CUSTOM_ScrHideTextDraw:
		CustomScrHideTextDraw = nullptr;
		break;
	case CUSTOM_ScrEditTextDraw:
		CustomScrEditTextDraw = nullptr;
		break;
	case CUSTOM_ScrTogglePlayerSpectating:
		CustomScrTogglePlayerSpectating = nullptr;
		break;
	case CUSTOM_GiveTakeDamage:
		CustomGiveTakeDamage = nullptr;
		break;
	case CUSTOM_RequestSpawn:
		CustomRequestSpawn = nullptr;
		break;
	case CUSTOM_EnterVehicle:
		CustomEnterVehicle = nullptr;
		break;
	case CUSTOM_ExitVehicle:
		CustomExitVehicle = nullptr;
		break;
	default:
		return false;
	}
	return true;
}

uint32_t CNetGame::GetNewThisID() {
	Retry:
	uint32_t id = getRandomNumber(0, 2147483647);
	for (uint32_t i : allThisID) {
		if (i == id)
			goto Retry;
	}
	allThisID.push_back(id);
	return id;
}

void CNetGame::RemoveThisID() {
	for (int i = 0; i < allThisID.size(); i++) {
		if (allThisID.at(i) == ThisID) {
			allThisID.erase(allThisID.begin() + i);
		}
	}
}


bool CNetGame::GoTo(POS target, VELOCITY speed, int animID) {

	if (this->GetGameState() != GAMESTATE_CONNECTED)
		return false;

	if (!this->iSpawned)
		return false;

	std::chrono::milliseconds interval(1000 / NETMODE_ONFOOT_SENDRATE);
	while (true) {
		std::this_thread::sleep_for(interval);

		float dX = target.x - this->getPos().x;
		float dY = target.y - this->getPos().y;
		float dZ = target.z - this->getPos().z;
		if (abs(dX) < 1 && abs(dY) < 1 && abs(dZ) < 0.1) {
			this->setAnimation(GetAnimationIndexFlag(1189, ANIMFLAG_NORMAL, 4));
			return true;
		}
		this->setAnimation(GetAnimationIndexFlag(animID, ANIMFLAG_NORMAL, 4));

		VELOCITY calVel = CalcVelocity(this->getPos(), target, speed, toPOS(1, 1, 0.1));
		this->fNormalModePos[0] += calVel.x;
		this->fNormalModePos[1] += calVel.y;
		this->fNormalModePos[2] += calVel.z;
		this->vecMoveSpeed[0] = calVel.x;
		this->vecMoveSpeed[1] = calVel.y;
		this->vecMoveSpeed[2] = calVel.z;
		QUATERNION q = eulerToQuaternion(PosToRotate(this->getPos(), target), 0, 0);
		this->setQuaternion(q);
	}
	return true;
}

bool CNetGame::GoToVehicle(POS target, VELOCITY speed)
{
	if (this->GetGameState() != GAMESTATE_CONNECTED)
		return false;

	if (!this->iSpawned)
		return false;

	if (!this->inVehicle)
		return false;

	std::chrono::milliseconds interval(1000 / NETMODE_INCAR_SENDRATE);
	while (true) {
		std::this_thread::sleep_for(interval);

		float dX = target.x - this->incarData->vecPos[0];
		float dY = target.y - this->incarData->vecPos[1];
		float dZ = target.z - this->incarData->vecPos[2];
		if (abs(dX) < 1 && abs(dY) < 1 && abs(dZ) < 0.1) {
			return true;
		}
		POS pos(this->incarData->vecPos[0], this->incarData->vecPos[1], this->incarData->vecPos[2]);
		VELOCITY calVel = CalcVelocity(pos, target, speed, toPOS(1, 1, 0.1));
		this->incarData->vecPos[0] += calVel.x;
		this->incarData->vecPos[1] += calVel.y;
		this->incarData->vecPos[2] += calVel.z;
		this->incarData->vecMoveSpeed[0] = calVel.x;
		this->incarData->vecMoveSpeed[1] = calVel.y;
		this->incarData->vecMoveSpeed[2] = calVel.z;
		QUATERNION q = eulerToQuaternion(PosToRotate(pos, target), 0, 0);
		this->incarData->fQuaternion[0] = q.x;
		this->incarData->fQuaternion[1] = q.y;
		this->incarData->fQuaternion[2] = q.z;
		this->incarData->fQuaternion[3] = q.w;
		std::cout << this->incarData->vecPos[0] << " " << this->incarData->vecPos[1] << " " << this->incarData->vecPos[2] << std::endl;
		std::cout << this->incarData->fQuaternion[0] << " " << this->incarData->fQuaternion[1] << " " << this->incarData->fQuaternion[2] << " " << this->incarData->fQuaternion[3] << std::endl;

	}
	return true;
}

void CNetGame::EnterVehicle(bool bPassenger, int iVehicleID, unsigned char seatFlags) {
	this->sInVehicle = this->vehiclesPool[iVehicleID];
	this->inVehicleID = iVehicleID;
	this->inVehicle = true;
	this->isPassenger = bPassenger;
	this->seatFlags = seatFlags;
	this->playersInfo[this->playerID].iAreWeInAVehicle = true;
	this->incarData->vecPos[0] = this->fullVehiclesPool[iVehicleID].vecPos[0];
	this->incarData->vecPos[1] = this->fullVehiclesPool[iVehicleID].vecPos[1];
	this->incarData->vecPos[2] = this->fullVehiclesPool[iVehicleID].vecPos[2];
	this->incarData->fCarHealth = this->fullVehiclesPool[iVehicleID].fHealth;
	SendEnterVehicleNotification(iVehicleID, bPassenger, this);
}

void CNetGame::ExitVehicle(int iVehicleID) {
	memset(&this->sInVehicle, 0, sizeof(stVehiclePool));
	this->inVehicleID = 0;
	this->inVehicle = false;
	this->isPassenger = false;
	this->seatFlags = 0;
	this->playersInfo[this->playerID].iAreWeInAVehicle = false;
	POS pos; pos.x = this->incarData->vecPos[0]; pos.y = this->incarData->vecPos[1]; pos.z = this->incarData->vecPos[2];
	this->setPos(pos);
	SendExitVehicleNotification(iVehicleID, this);
}