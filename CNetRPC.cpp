#include "CNetRPC.h"

#include <sstream>

#define CUSTOM(var, name) (var##name)

std::unordered_map<int, CNetRPC*> CNetRPC::rpcInstances;

CNetRPC::CNetRPC(CNetGame* netgame, int instanceID) {
	this->instanceID = instanceID;
	this->netgame = netgame;
}

std::string GetName(std::string str) {
	std::string methodName = str;
	size_t pos = methodName.find("::");
	if (pos != std::string::npos) {
		methodName = methodName.substr(pos + 2);
	}
	return methodName;
}

void CNetRPC::ServerJoin(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CHAR szPlayerName[256];
	PLAYERID playerId;
	BYTE byteNameLen = 0;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(playerId);
	int iUnk = 0;
	bsData.Read(iUnk);
	BYTE bIsNPC = 0;
	bsData.Read(bIsNPC);
	bsData.Read(byteNameLen);
	if (byteNameLen > 20) return;
	bsData.Read(szPlayerName, byteNameLen);
	szPlayerName[byteNameLen] = '\0';

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	netgame->playersInfo[playerId].iIsConnected = 1;
	netgame->playersInfo[playerId].byteIsNPC = bIsNPC;
	strcpy((char*)netgame->playersInfo[playerId].szPlayerName, szPlayerName);

	if (netgame->CustomServerJoin != nullptr)
		netgame->CustomServerJoin(netgame, rpcParams);
}

void CNetRPC::ServerQuit(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	BYTE byteReason;

	bsData.Read(playerId);
	bsData.Read(byteReason);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	netgame->playersInfo[playerId].iIsConnected = 0;
	netgame->playersInfo[playerId].byteIsNPC = 0;

	memset(netgame->playersInfo[playerId].szPlayerName, 0, 20);
	 
	if (netgame->CustomServerQuit != nullptr)
		netgame->CustomServerQuit(netgame, rpcParams);
}

void CNetRPC::InitGame(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsInitGame((unsigned char*)data, (iBitLength/8)+1, false);

	bool bLanMode, bStuntBonus;
	BYTE byteVehicleModels[212];

	bsInitGame.ReadCompressed(netgame->m_bZoneNames);
	bsInitGame.ReadCompressed(netgame->m_bUseCJWalk);
	bsInitGame.ReadCompressed(netgame->m_bAllowWeapons);
	bsInitGame.ReadCompressed(netgame->m_bLimitGlobalChatRadius);
	bsInitGame.Read(netgame->m_fGlobalChatRadius);
	bsInitGame.ReadCompressed(bStuntBonus);
	bsInitGame.Read(netgame->m_fNameTagDrawDistance);
	bsInitGame.ReadCompressed(netgame->m_bDisableEnterExits);
	bsInitGame.ReadCompressed(netgame->m_bNameTagLOS);
	bsInitGame.ReadCompressed(netgame->m_bManualVehicleEngineAndLight);
	bsInitGame.Read(netgame->m_iSpawnsAvailable);
	bsInitGame.Read(netgame->playerID);
	bsInitGame.ReadCompressed(netgame->m_bShowPlayerTags);
	bsInitGame.Read(netgame->m_iShowPlayerMarkers);
	bsInitGame.Read(netgame->m_byteWorldTime);
	bsInitGame.Read(netgame->m_byteWeather);
	bsInitGame.Read(netgame->m_fGravity);
	bsInitGame.ReadCompressed(bLanMode);
	bsInitGame.Read(netgame->m_iDeathDropMoney);
	bsInitGame.ReadCompressed(netgame->m_bInstagib);

	bsInitGame.Read(netgame->iNetModeNormalOnfootSendRate);
	bsInitGame.Read(netgame->iNetModeNormalInCarSendRate);
	bsInitGame.Read(netgame->iNetModeFiringSendRate);
	bsInitGame.Read(netgame->iNetModeSendMultiplier);

	bsInitGame.Read(netgame->m_iLagCompensation);

	uint8_t byteStrLen;
	bsInitGame.Read(byteStrLen);
	if (byteStrLen) {
		memset(netgame->m_szHostName, 0, sizeof(netgame->m_szHostName));
		bsInitGame.Read(netgame->m_szHostName, byteStrLen);
	}
	netgame->m_szHostName[byteStrLen] = '\0';

	bsInitGame.Read((char*)&byteVehicleModels[0], 212);
	bsInitGame.Read(netgame->m_iVehicleFrendlyFire);

	netgame->incarData = &netgame->playersInfo[netgame->playerID].incarData;

	netgame->SetGameState(GAMESTATE_CONNECTED);
	 
	if (netgame->CustomInitGame != nullptr)
		netgame->CustomInitGame(netgame, rpcParams);
}

void CNetRPC::WorldPlayerAdd(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	PLAYERID playerId;
	BYTE byteFightingStyle = 4;
	BYTE byteTeam = 0;
	int iSkin = 0;
	float vecPos[3];
	float fRotation = 0;
	DWORD dwColor = 0;

	bsData.Read(playerId);
	bsData.Read(byteTeam);
	bsData.Read(iSkin);
	bsData.Read(vecPos[0]);
	bsData.Read(vecPos[1]);
	bsData.Read(vecPos[2]);
	bsData.Read(fRotation);
	bsData.Read(dwColor);
	bsData.Read(byteFightingStyle);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	netgame->playersInfo[playerId].iIsStreamedIn = 1;
	netgame->playersInfo[playerId].onfootData.vecPos[0] =
		netgame->playersInfo[playerId].incarData.vecPos[0] = vecPos[0];
	netgame->playersInfo[playerId].onfootData.vecPos[1] =
		netgame->playersInfo[playerId].incarData.vecPos[1] = vecPos[1];
	netgame->playersInfo[playerId].onfootData.vecPos[2] =
		netgame->playersInfo[playerId].incarData.vecPos[2] = vecPos[2];
	 
	if (netgame->CustomWorldPlayerAdd != nullptr)
		netgame->CustomWorldPlayerAdd(netgame, rpcParams);
}

void CNetRPC::WorldPlayerDeath(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	PLAYERID playerId;
	bsData.Read(playerId);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	if (netgame->CustomWorldPlayerDeath != nullptr)
		netgame->CustomWorldPlayerDeath(netgame, rpcParams);
}

void CNetRPC::WorldPlayerRemove(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	PLAYERID playerId = 0;
	bsData.Read(playerId);

	if (playerId < 0 || playerId >= MAX_PLAYERS) return;

	netgame->playersInfo[playerId].iIsStreamedIn = 0;
	netgame->playersInfo[playerId].incarData.vecPos[0] = 0.0f;
	netgame->playersInfo[playerId].incarData.vecPos[1] = 0.0f;
	netgame->playersInfo[playerId].incarData.vecPos[2] = 0.0f;
	 
	if (netgame->CustomWorldPlayerRemove != nullptr)
		netgame->CustomWorldPlayerRemove(netgame, rpcParams);
}

void CNetRPC::WorldVehicleAdd(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	NEW_VEHICLE NewVehicle;

	bsData.Read((char*)&NewVehicle, sizeof(NEW_VEHICLE));

	if (NewVehicle.VehicleId < 0 || NewVehicle.VehicleId >= MAX_VEHICLES) return;

	netgame->vehiclesPool[NewVehicle.VehicleId].iDoesExist = 1;
	netgame->vehiclesPool[NewVehicle.VehicleId].fPos[0] = NewVehicle.vecPos[0];
	netgame->vehiclesPool[NewVehicle.VehicleId].fPos[1] = NewVehicle.vecPos[1];
	netgame->vehiclesPool[NewVehicle.VehicleId].fPos[2] = NewVehicle.vecPos[2];
	netgame->vehiclesPool[NewVehicle.VehicleId].iModelID = NewVehicle.iVehicleType;
	netgame->fullVehiclesPool[NewVehicle.VehicleId] = NewVehicle;
	 
	if (netgame->CustomWorldVehicleAdd != nullptr)
		netgame->CustomWorldVehicleAdd(netgame, rpcParams);
}

void CNetRPC::WorldVehicleRemove(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID VehicleID;

	bsData.Read(VehicleID);

	if (VehicleID < 0 || VehicleID >= MAX_VEHICLES) return;

	netgame->vehiclesPool[VehicleID].iDoesExist = 0;
	netgame->vehiclesPool[VehicleID].fPos[0] = 0.0f;
	netgame->vehiclesPool[VehicleID].fPos[1] = 0.0f;
	netgame->vehiclesPool[VehicleID].fPos[2] = 0.0f;
	memset(&netgame->fullVehiclesPool[VehicleID], 0, sizeof(NEW_VEHICLE));
	 
	if (netgame->CustomWorldVehicleRemove != nullptr)
		netgame->CustomWorldVehicleRemove(netgame, rpcParams);
}

void CNetRPC::ConnectionRejected(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteRejectReason;

	bsData.Read(byteRejectReason);

	if (byteRejectReason == REJECT_REASON_BAD_VERSION)
	{
		Log("[SAMP] Bad SA-MP version.");
	}
	else if (byteRejectReason == REJECT_REASON_BAD_NICKNAME)
	{
		Log("[SAMP] Bad nickname.");
	}
	else if (byteRejectReason == REJECT_REASON_BAD_MOD)
	{
		Log("[SAMP] Bad mod version.");
	}
	else if (byteRejectReason == REJECT_REASON_BAD_PLAYERID)
	{
		Log("[SAMP] Bad player ID.");
	}
	else
		Log("ConnectionRejected: unknown");

	if (netgame->CustomConnectionRejected != nullptr)
		netgame->CustomConnectionRejected(netgame, rpcParams);
	 
}

void CNetRPC::ClientMessage(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	DWORD dwStrLen, dwColor;
	char szMsg[257];
	memset(szMsg, 0, 257);

	bsData.Read(dwColor);
	bsData.Read(dwStrLen);
	if (dwStrLen > 256) return;

	bsData.Read(szMsg, dwStrLen);
	szMsg[dwStrLen] = 0;

	char szNonColorEmbeddedMsg[257];
	int iNonColorEmbeddedMsgLen = 0;

	for (size_t pos = 0; pos < strlen(szMsg) && szMsg[pos] != '\0'; pos++)
	{
		if (!((*(unsigned char*)(&szMsg[pos]) - 32) >= 0 && (*(unsigned char*)(&szMsg[pos]) - 32) < 224))
			continue;

		if (pos + 7 < strlen(szMsg))
		{
			if (szMsg[pos] == '{' && szMsg[pos + 7] == '}')
			{
				pos += 7;
				continue;
			}
		}

		szNonColorEmbeddedMsg[iNonColorEmbeddedMsgLen] = szMsg[pos];
		iNonColorEmbeddedMsgLen++;
	}

	szNonColorEmbeddedMsg[iNonColorEmbeddedMsgLen] = 0;
	 
	netgame->vClientMessages.push_back(szNonColorEmbeddedMsg);

	if (netgame->CustomClientMessage != nullptr)
		netgame->CustomClientMessage(netgame, rpcParams);
}

void CNetRPC::Chat(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	PlayerID sender = rpcParams->sender;

	RakNet::BitStream bsData((unsigned char *)Data,(iBitLength/8)+1,false);
	PLAYERID playerId;
	BYTE byteTextLen;

	char szText[256];
	memset(szText, 0, 256);

	bsData.Read(playerId);
	bsData.Read(byteTextLen);
	bsData.Read((char*)szText, byteTextLen);
	szText[byteTextLen] = '\0';

	if(playerId < 0 || playerId >= MAX_PLAYERS)
		return;
	
	netgame->vChatMessages.push_back(szText);

	if (netgame->CustomChat != nullptr)
		netgame->CustomChat(netgame, rpcParams);
}

void CNetRPC::UpdateScoresPingsIPs(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	PLAYERID playerId;
	int iPlayerScore;
	DWORD dwPlayerPing;

	for (PLAYERID i = 0; i < (iBitLength / 8) / 9; i++)
	{
		bsData.Read(playerId);
		bsData.Read(iPlayerScore);
		bsData.Read(dwPlayerPing);

		if (playerId < 0 || playerId >= MAX_PLAYERS)
			continue;

		netgame->playersInfo[playerId].iScore = iPlayerScore;
		netgame->playersInfo[playerId].dwPing = dwPlayerPing;
	}
	 
	if (netgame->CustomUpdateScoresPingsIPs != nullptr)
		netgame->CustomUpdateScoresPingsIPs(netgame, rpcParams);
}

void CNetRPC::SetCheckpoint(RPCParameters* rpcParams) {
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(netgame->fCurrentCheckpoint[0]);
	bsData.Read(netgame->fCurrentCheckpoint[1]);
	bsData.Read(netgame->fCurrentCheckpoint[2]);
	bsData.Read(netgame->fSizeCheckpoint);

	netgame->bActiveCheckpoint = true;

	if (netgame->CustomSetCheckpoint != nullptr)
		netgame->CustomSetCheckpoint(netgame, rpcParams);
}

void CNetRPC::DisableCheckpoint(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	netgame->bActiveCheckpoint = false;

	if (netgame->CustomDisableCheckpoint != nullptr)
		netgame->CustomDisableCheckpoint(netgame, rpcParams);
}

void CNetRPC::Pickup(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int PickupID;
	PICKUP Pickup;

	bsData.Read(PickupID);
	bsData.Read((PCHAR)&Pickup, sizeof(PICKUP));

	PICKUP_PACK pack;
	pack.PickupID = PickupID;
	pack.Pickup = Pickup;
	netgame->vPickupInfo.push_back(pack);

	if (netgame->CustomPickup != nullptr)
		netgame->CustomPickup(netgame, rpcParams);
}

void CNetRPC::DestroyPickup(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int PickupID;

	bsData.Read(PickupID);

	for (unsigned i = 0; i < netgame->vPickupInfo.size(); i++) {
		if (netgame->vPickupInfo.at(i).PickupID == PickupID) {
			netgame->vPickupInfo.erase(netgame->vPickupInfo.begin() + i);
			continue;
		}
	}

	if (netgame->CustomDestroyPickup != nullptr)
		netgame->CustomDestroyPickup(netgame, rpcParams);
}

void CNetRPC::RequestClass(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteRequestOutcome = 0;

	bsData.Read(byteRequestOutcome);
	bsData.Read((PCHAR)&netgame->SpawnInfo, sizeof(PLAYER_SPAWN_INFO));
	std::cout << netgame->SpawnInfo.vecPos[0] << " ";
	std::cout << netgame->SpawnInfo.vecPos[1] << " ";
	std::cout << netgame->SpawnInfo.vecPos[2] << std::endl;
	 
	if (netgame->CustomRequestClass != nullptr)
		netgame->CustomRequestClass(netgame, rpcParams);
}

void CNetRPC::ScrInitMenu(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	memset(&netgame->GTAMenu, 0, sizeof(struct stGTAMenu));

	BYTE byteMenuID;
	BOOL bColumns; // 0 = 1, 1 = 2
	CHAR cText[MAX_MENU_LINE];
	float fX;
	float fY;
	float fCol1;
	float fCol2 = 0.0;
	MENU_INT MenuInteraction;

	bsData.Read(byteMenuID);
	bsData.Read(bColumns);
	bsData.Read(cText, MAX_MENU_LINE);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fCol1);
	if (bColumns) bsData.Read(fCol2);
	bsData.Read(MenuInteraction.bMenu);
	for (BYTE i = 0; i < MAX_MENU_ITEMS; i++)
		bsData.Read(MenuInteraction.bRow[i]);

	//Log("[MENU] %s", cText);
	strcpy(netgame->GTAMenu.szTitle, cText);

	BYTE byteColCount;
	bsData.Read(cText, MAX_MENU_LINE);
	//Log("[MENU] %s", cText);
	strcpy(netgame->GTAMenu.szSeparator, cText);

	bsData.Read(byteColCount);
	netgame->GTAMenu.byteColCount = byteColCount;
	for (BYTE i = 0; i < byteColCount; i++)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		//Log("[MENU:%d] %s", i, cText);
		strcpy(netgame->GTAMenu.szColumnContent[i], cText);
	}

	if (bColumns)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		//Log("4: %s", cText);

		bsData.Read(byteColCount);
		for (BYTE i = 0; i < byteColCount; i++)
		{
			bsData.Read(cText, MAX_MENU_LINE);
			//Log("5: %d %s", i, cText);
		}
	}

	if (netgame->CustomScrInitMenu != nullptr)
		netgame->CustomScrInitMenu(netgame, rpcParams);
}

void CNetRPC::ScrDialogBox(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	if (netgame->GetGameState() != GAMESTATE_CONNECTED)
		return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(netgame->sampDialog.wDialogID);
	bsData.Read(netgame->sampDialog.bDialogStyle);

	bsData.Read(netgame->sampDialog.bTitleLength);
	bsData.Read(netgame->sampDialog.szTitle, netgame->sampDialog.bTitleLength);
	netgame->sampDialog.szTitle[netgame->sampDialog.bTitleLength] = 0;

	bsData.Read(netgame->sampDialog.bButton1Len);
	bsData.Read(netgame->sampDialog.szButton1, netgame->sampDialog.bButton1Len);
	netgame->sampDialog.szButton1[netgame->sampDialog.bButton1Len] = 0;

	bsData.Read(netgame->sampDialog.bButton2Len);
	bsData.Read(netgame->sampDialog.szButton2, netgame->sampDialog.bButton2Len);
	netgame->sampDialog.szButton2[netgame->sampDialog.bButton2Len] = 0;

	//stringCompressor->DecodeString(sampDialog.szInfo, 256, &bsData);

	switch (netgame->sampDialog.bDialogStyle)
	{
	case DIALOG_STYLE_MSGBOX:
		if (!netgame->sampDialog.iIsActive)
			netgame->sampDialog.iIsActive = 1;
		break;

	case DIALOG_STYLE_INPUT:
		if (!netgame->sampDialog.iIsActive)
			netgame->sampDialog.iIsActive = 1;
		break;

	case DIALOG_STYLE_LIST:
		if (!netgame->sampDialog.iIsActive)
			netgame->sampDialog.iIsActive = 1;
		break;

	case DIALOG_STYLE_PASSWORD:
		if (!netgame->sampDialog.iIsActive)
			netgame->sampDialog.iIsActive = 1;
		break;

	default:
		if (netgame->sampDialog.iIsActive)
			netgame->sampDialog.iIsActive = 0;

		break;
	}
	 
	if (netgame->CustomScrDialogBox != nullptr)
		netgame->CustomScrDialogBox(netgame, rpcParams);
}

void CNetRPC::ScrGameText(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	char szMessage[400];
	int iType, iTime, iLength;

	bsData.Read(iType);
	bsData.Read(iTime);
	bsData.Read(iLength);

	if (iLength > 400) return; // tsk tsk, kye

	bsData.Read(szMessage, iLength);
	szMessage[iLength] = '\0';
	 
	if (netgame->CustomScrGameText != nullptr)
		netgame->CustomScrGameText(netgame, rpcParams);
}

void CNetRPC::ScrPlayAudioStream(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	unsigned char bURLLen;
	char szURL[256];

	bsData.Read(bURLLen);
	bsData.Read(szURL, bURLLen);
	szURL[bURLLen] = 0;
	 
	if (netgame->CustomScrPlayAudioStream != nullptr)
		netgame->CustomScrPlayAudioStream(netgame, rpcParams);
}

void CNetRPC::ScrSetDrunkLevel(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(netgame->iDrunkLevel);
	 
	if (netgame->CustomScrSetDrunkLevel != nullptr)
		netgame->CustomScrSetDrunkLevel(netgame, rpcParams);
}

void CNetRPC::ScrHaveSomeMoney(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int iGivenMoney;
	bsData.Read(iGivenMoney);

	netgame->iMoney += iGivenMoney;
	 
	if (netgame->CustomScrHaveSomeMoney != nullptr)
		netgame->CustomScrHaveSomeMoney(netgame, rpcParams);
}

void CNetRPC::ScrResetMoney(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	netgame->iMoney = 0;

	if (netgame->CustomScrResetMoney != nullptr)
		netgame->CustomScrResetMoney(netgame, rpcParams);
}

void CNetRPC::ScrSetPlayerPos(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(netgame->fNormalModePos[0]);
	bsData.Read(netgame->fNormalModePos[1]);
	bsData.Read(netgame->fNormalModePos[2]);
	 
	if (netgame->CustomScrSetPlayerPos != nullptr)
		netgame->CustomScrSetPlayerPos(netgame, rpcParams);
}

void CNetRPC::ScrSetPlayerFacingAngle(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bsData.Read(netgame->fNormalModeRot);
	 
	if (netgame->CustomScrSetFacingAngle != nullptr)
		netgame->CustomScrSetFacingAngle(netgame, rpcParams);
}

void CNetRPC::ScrSetSpawnInfo(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	PLAYER_SPAWN_INFO SpawnInfo;

	bsData.Read((PCHAR)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	netgame->fNormalModePos[0] = SpawnInfo.vecPos[0];
	netgame->fNormalModePos[1] = SpawnInfo.vecPos[1];
	netgame->fNormalModePos[2] = SpawnInfo.vecPos[2];
	netgame->SpawnInfo = SpawnInfo;
	 
	if (netgame->CustomScrSetSpawnInfo != nullptr)
		netgame->CustomScrSetSpawnInfo(netgame, rpcParams);
}

void CNetRPC::ScrSetPlayerHealth(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	if (!netgame->bInfinityHealth)
		bsData.Read(netgame->fPlayerHealth);
	 
	if (netgame->CustomScrSetPlayerHealth != nullptr)
		netgame->CustomScrSetPlayerHealth(netgame, rpcParams);
}

void CNetRPC::ScrSetPlayerArmour(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	if (!netgame->bInfinityArmour) 
		bsData.Read(netgame->fPlayerArmour);

	if (netgame->CustomScrSetPlayerArmour != nullptr)
		netgame->CustomScrSetPlayerArmour(netgame, rpcParams);
}

void CNetRPC::ScrSetPlayerSkin(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int iPlayerID;
	unsigned int uiSkin;

	bsData.Read(iPlayerID);
	bsData.Read(uiSkin);

	if (iPlayerID < 0 || iPlayerID >= MAX_PLAYERS)
		return;

	if (netgame->GetGameState() == GAMESTATE_CONNECTED && netgame->playerID == iPlayerID)
		netgame->iLocalPlayerSkin = uiSkin;

	if (netgame->CustomScrSetPlayerSkin != nullptr)
			netgame->CustomScrSetPlayerSkin(netgame, rpcParams);
}

void CNetRPC::ScrCreateObject(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	unsigned short ObjectID;
	bsData.Read(ObjectID);

	unsigned long ModelID;
	bsData.Read(ModelID);

	float vecPos[3];
	bsData.Read(vecPos[0]);
	bsData.Read(vecPos[1]);
	bsData.Read(vecPos[2]);

	float vecRot[3];
	bsData.Read(vecRot[0]);
	bsData.Read(vecRot[1]);
	bsData.Read(vecRot[2]);

	float fDrawDistance;
	bsData.Read(fDrawDistance);

	/*if (settings.uiObjectsLogging != 0)
	{
		char szCreateObjectAlert[256];
		sprintf_s(szCreateObjectAlert, sizeof(szCreateObjectAlert), "[OBJECT] %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.2f", ObjectID, ModelID, vecPos[0], vecPos[1], vecPos[2], vecRot[0], vecRot[1], vecRot[2], fDrawDistance);
		Log(szCreateObjectAlert);
	}*/

	if (netgame->CustomScrCreateObject != nullptr)
		netgame->CustomScrCreateObject(netgame, rpcParams);
}

void CNetRPC::ScrCreate3DTextLabel(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	WORD ID;
	CHAR Text[256];
	DWORD dwColor;
	FLOAT vecPos[3];
	FLOAT DrawDistance;
	BYTE UseLOS;
	WORD PlayerID;
	WORD VehicleID;

	bsData.Read((WORD)ID);
	bsData.Read((DWORD)dwColor);
	bsData.Read((FLOAT)vecPos[0]);
	bsData.Read((FLOAT)vecPos[1]);
	bsData.Read((FLOAT)vecPos[2]);
	bsData.Read((FLOAT)DrawDistance);
	bsData.Read((BYTE)UseLOS);
	bsData.Read((WORD)PlayerID);
	bsData.Read((WORD)VehicleID);

	/*stringCompressor->DecodeString(Text, 256, &bsData);

	if (settings.uiTextLabelsLogging != 0)
	{
		char szCreate3DTextLabelAlert[256];
		sprintf_s(szCreate3DTextLabelAlert, sizeof(szCreate3DTextLabelAlert), "[TEXTLABEL] %d - %s (%X, %.3f, %.3f, %.3f, %.2f, %i, %d, %d)", ID, Text, dwColor, vecPos[0], vecPos[1], vecPos[2], DrawDistance, UseLOS, PlayerID, VehicleID);
		Log(szCreate3DTextLabelAlert);
	}*/

	if (netgame->CustomScrCreate3DTextLabel != nullptr)
		netgame->CustomScrCreate3DTextLabel(netgame, rpcParams);
}

void CNetRPC::ScrShowTextDraw(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	WORD wTextID;
	TEXT_DRAW_TRANSMIT TextDrawTransmit;

	CHAR cText[1024];
	unsigned short cTextLen = 0;

	bsData.Read(wTextID);
	bsData.Read((PCHAR)&TextDrawTransmit, sizeof(TEXT_DRAW_TRANSMIT));
	bsData.Read(cTextLen);
	bsData.Read(cText, cTextLen);
	cText[cTextLen] = '\0';

	/*if (settings.uiTextDrawsLogging != 0)
		SaveTextDrawData(wTextID, &TextDrawTransmit, cText);

	if (TextDrawTransmit.byteSelectable)
		Log("[SELECTABLE-TEXTDRAW] ID: %d, Text: %s.", wTextID, cText);*/

	if (netgame->CustomScrShowTextDraw != nullptr)
		netgame->CustomScrShowTextDraw(netgame, rpcParams);
}

void CNetRPC::ScrHideTextDraw(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	WORD wTextID;
	bsData.Read(wTextID);

	/*if (settings.uiTextDrawsLogging != 0)
		Log("[TEXTDRAW:HIDE] ID: %d.", wTextID);*/

	if (netgame->CustomScrHideTextDraw != nullptr)
		netgame->CustomScrHideTextDraw(netgame, rpcParams);
}

void CNetRPC::ScrEditTextDraw(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	WORD wTextID;
	CHAR cText[1024];
	unsigned short cTextLen = 0;

	bsData.Read(wTextID);
	bsData.Read(cTextLen);
	bsData.Read(cText, cTextLen);
	cText[cTextLen] = '\0';

	/*if (settings.uiTextDrawsLogging != 0)
		Log("[TEXTDRAW:EDIT] ID: %d, Text: %s.", wTextID, cText);*/

	if (netgame->CustomScrEditTextDraw != nullptr)
		netgame->CustomScrEditTextDraw(netgame, rpcParams);
}

void CNetRPC::ScrTogglePlayerSpectating(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BOOL bToggle;

	bsData.Read(bToggle);

	if (netgame->bIsSpectating && !bToggle && !netgame->iSpawned)
	{
		sampSpawn(netgame);
		netgame->iSpawned = 1;
	}

	netgame->bIsSpectating = bToggle;

	if (netgame->CustomScrTogglePlayerSpectating != nullptr)
		netgame->CustomScrTogglePlayerSpectating(netgame, rpcParams);
}

void CNetRPC::GiveTakeDamage(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BOOL bGiveTake;
	uint16_t PlayerID;
	float fDamage;
	uint32_t uiWeaponID, uiBodyTake;
	bsData.Read(bGiveTake);
	bsData.Read(PlayerID);
	bsData.Read(fDamage);
	bsData.Read(uiWeaponID);
	bsData.Read(uiBodyTake);
	if (!netgame->bInfinityHealth)
		netgame->fPlayerHealth -= fDamage;

	if (netgame->CustomGiveTakeDamage != nullptr)
		netgame->CustomGiveTakeDamage(netgame, rpcParams);
}

void CNetRPC::EnterVehicle(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	VEHICLEID VehicleID = 0;
	uint8_t bytePassenger = 0;
	bool bPassenger = false;

	bsData.Read(playerId);
	bsData.Read(VehicleID);
	bsData.Read(bytePassenger);

	if (bytePassenger) bPassenger = true;

	if (netgame->inVehicle && VehicleID == netgame->inVehicleID) {
		if (bPassenger) {
			bool search = false;
			for (int i = 0; i < netgame->PassengerID.size(); i++) {
				int player = netgame->PassengerID.at(i);
				if (player == playerId) {
					search = true;
					break;
				}
			}
			if (!search) {
				netgame->PassengerID.push_back(playerId);
			}
		}
	}

	if (netgame->CustomEnterVehicle != nullptr)
		netgame->CustomEnterVehicle(netgame, rpcParams);
}

void CNetRPC::ExitVehicle(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	VEHICLEID VehicleID = 0;

	bsData.Read(playerId);
	bsData.Read(VehicleID);

	if (netgame->inVehicle && VehicleID == netgame->inVehicleID) {
		for (int i = 0; i < netgame->PassengerID.size(); i++) {
			int player = netgame->PassengerID.at(i);
			if (player == playerId) {
				netgame->PassengerID.erase(netgame->PassengerID.begin() + i);
			}
		}
	}

	if (netgame->CustomExitVehicle != nullptr)
		netgame->CustomExitVehicle(netgame, rpcParams);
}

void CNetRPC::RequestSpawn(RPCParameters* rpcParams)
{
	Log(__FUNCTION__);
	CNetRPC* thiz = rpcInstances.at(rpcParams->customID);
	CNetGame* netgame = thiz->netgame;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE bRequestResponse;
	bsData.Read(bRequestResponse);

	netgame->bSpawnRequest = bRequestResponse;

	if (netgame->CustomRequestSpawn != nullptr)
		netgame->CustomRequestSpawn(netgame, rpcParams);
}

void CNetRPC::RegisterRPC(RakClientInterface* client) {
	rpcInstances[this->instanceID] = this;
	if (client) {
		// Core RPCs
		client->RegisterAsRemoteProcedureCall(&RPC_ServerJoin, ServerJoin, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ServerQuit, ServerQuit, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_InitGame, InitGame, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerAdd, WorldPlayerAdd, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerDeath, WorldPlayerDeath, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerRemove, WorldPlayerRemove, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_WorldVehicleAdd, WorldVehicleAdd, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_WorldVehicleRemove, WorldVehicleRemove, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ConnectionRejected, ConnectionRejected, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ClientMessage, ClientMessage, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_Chat, Chat, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_UpdateScoresPingsIPs, UpdateScoresPingsIPs, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_SetCheckpoint, SetCheckpoint, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_DisableCheckpoint, DisableCheckpoint, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_Pickup, Pickup, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_DestroyPickup, DestroyPickup, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_RequestClass, RequestClass, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_PlayerGiveTakeDamage, GiveTakeDamage, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_RequestSpawn, RequestSpawn, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_EnterVehicle, EnterVehicle, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ExitVehicle, ExitVehicle, &this->instanceID);

		// Scripting RPCs
		client->RegisterAsRemoteProcedureCall(&RPC_ScrInitMenu, ScrInitMenu, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrDialogBox, ScrDialogBox, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrDisplayGameText, ScrGameText, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_PlayAudioStream, ScrPlayAudioStream, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkLevel, ScrSetDrunkLevel, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrHaveSomeMoney, ScrHaveSomeMoney, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrResetMoney, ScrResetMoney, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPos, ScrSetPlayerPos, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerFacingAngle, ScrSetPlayerFacingAngle, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetSpawnInfo, ScrSetSpawnInfo, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerHealth, ScrSetPlayerHealth, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerArmour, ScrSetPlayerArmour, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkin, ScrSetPlayerSkin, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrCreateObject, ScrCreateObject, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrCreate3DTextLabel, ScrCreate3DTextLabel, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrShowTextDraw, ScrShowTextDraw, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrHideTextDraw, ScrHideTextDraw, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrEditTextDraw, ScrEditTextDraw, &this->instanceID);
		client->RegisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerSpectating, ScrTogglePlayerSpectating, &this->instanceID);
	}
}

void CNetRPC::UnRegisterRPC(RakClientInterface* client) {
	if (client)
	{
		// Core RPCs
		client->UnregisterAsRemoteProcedureCall(&RPC_ServerJoin);
		client->UnregisterAsRemoteProcedureCall(&RPC_ServerQuit);
		client->UnregisterAsRemoteProcedureCall(&RPC_InitGame);
		client->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerAdd);
		client->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerDeath);
		client->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerRemove);
		client->UnregisterAsRemoteProcedureCall(&RPC_WorldVehicleAdd);
		client->UnregisterAsRemoteProcedureCall(&RPC_WorldVehicleRemove);
		client->UnregisterAsRemoteProcedureCall(&RPC_ConnectionRejected);
		client->UnregisterAsRemoteProcedureCall(&RPC_ClientMessage);
		client->UnregisterAsRemoteProcedureCall(&RPC_Chat);
		client->UnregisterAsRemoteProcedureCall(&RPC_UpdateScoresPingsIPs);
		client->UnregisterAsRemoteProcedureCall(&RPC_SetCheckpoint);
		client->UnregisterAsRemoteProcedureCall(&RPC_DisableCheckpoint);
		client->UnregisterAsRemoteProcedureCall(&RPC_Pickup);
		client->UnregisterAsRemoteProcedureCall(&RPC_DestroyPickup);
		client->UnregisterAsRemoteProcedureCall(&RPC_RequestClass);

		// Scripting RPCs
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrInitMenu);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrDialogBox);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrDisplayGameText);
		client->UnregisterAsRemoteProcedureCall(&RPC_PlayAudioStream);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerDrunkLevel);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrHaveSomeMoney);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrResetMoney);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPos);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerFacingAngle);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetSpawnInfo);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerHealth);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerArmour);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkin);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrCreateObject);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrCreate3DTextLabel);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrShowTextDraw);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrHideTextDraw);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrEditTextDraw);
		client->UnregisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerSpectating);
	}
}
