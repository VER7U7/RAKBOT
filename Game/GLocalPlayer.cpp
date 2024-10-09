#include <cstdint>
#include "GLocalPlayer.h"

int iFollowingPassenger = 0, iFollowingDriver = 0;
void SendOnFootFullSyncData(ONFOOT_SYNC_DATA* pofSync, int sendDeathNoti, PLAYERID followPlayerID, CNetGame* netgame)
{
	if (pofSync == NULL)
		return;

	RakNet::BitStream bsPlayerSync;

	if (followPlayerID != (PLAYERID)-1)
	{

	}
	else
	{
		pofSync->lrAnalog = netgame->lrAnalog;
		pofSync->udAnalog = netgame->udAnalog;
		pofSync->wKeys = netgame->usKeys;
		pofSync->vecPos[0] = netgame->fNormalModePos[0];
		pofSync->vecPos[1] = netgame->fNormalModePos[1];
		pofSync->vecPos[2] = netgame->fNormalModePos[2];
		pofSync->fQuaternion[0] = netgame->fQuaternion[0];
		pofSync->fQuaternion[1] = netgame->fQuaternion[1];
		pofSync->fQuaternion[2] = netgame->fQuaternion[2];
		pofSync->fQuaternion[3] = netgame->fQuaternion[3];
		pofSync->byteHealth = netgame->fPlayerHealth;
		pofSync->byteArmour = netgame->fPlayerArmour;

		uint8_t exKeys = 1;
		pofSync->byteCurrentWeapon = (exKeys << 6) | pofSync->byteCurrentWeapon & 0x3F;
		pofSync->byteCurrentWeapon ^= (pofSync->byteCurrentWeapon ^ netgame->bCurrentWeapon) & 0x3F;

		pofSync->byteSpecialAction = netgame->byteSpecialAction;
		pofSync->vecMoveSpeed[0] = netgame->vecMoveSpeed[0];
		pofSync->vecMoveSpeed[1] = netgame->vecMoveSpeed[1];
		pofSync->vecMoveSpeed[2] = netgame->vecMoveSpeed[2];
		pofSync->vecSurfOffsets[0] = netgame->vecSurfOffsets[0];
		pofSync->vecSurfOffsets[1] = netgame->vecSurfOffsets[1];
		pofSync->vecSurfOffsets[2] = netgame->vecSurfOffsets[2];

		pofSync->wSurfInfo = netgame->usSurfInfo;
		pofSync->uiAnimation = netgame->uiAnimation;

		bsPlayerSync.Write((uint8_t)ID_PLAYER_SYNC);
		bsPlayerSync.Write((char*)pofSync, sizeof(ONFOOT_SYNC_DATA));

		netgame->client->Send(&bsPlayerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		if (sendDeathNoti && pofSync->byteHealth == 0)
			SendWastedNotification(0, -1, netgame);

		netgame->dwLastOnFootDataSentTick = netgame->GetTickCount();
	}
}

void SendInCarFullSyncData(INCAR_SYNC_DATA* picSync, int iVehicleID, CNetGame* netgame)
{
	RakNet::BitStream bsVehicleSync;

	if (!netgame->vehiclesPool[iVehicleID].iDoesExist)
		return;
	if (iVehicleID == INVALID_VEHICLE_ID) return;

	picSync->VehicleID = iVehicleID;
	picSync->lrAnalog = netgame->lrAnalog;
	picSync->udAnalog = netgame->udAnalog;
	picSync->wKeys = netgame->usKeys;
	if (netgame->incarData != nullptr) {
		picSync->fQuaternion[0] = netgame->incarData->fQuaternion[0];
		picSync->fQuaternion[1] = netgame->incarData->fQuaternion[1];
		picSync->fQuaternion[2] = netgame->incarData->fQuaternion[2];
		picSync->fQuaternion[3] = netgame->incarData->fQuaternion[3];

		picSync->vecPos[0] = netgame->incarData->vecPos[0];
		picSync->vecPos[1] = netgame->incarData->vecPos[1];
		picSync->vecPos[2] = netgame->incarData->vecPos[2];

		picSync->vecMoveSpeed[0] = netgame->incarData->vecMoveSpeed[0];
		picSync->vecMoveSpeed[1] = netgame->incarData->vecMoveSpeed[1];
		picSync->vecMoveSpeed[2] = netgame->incarData->vecMoveSpeed[2];

		picSync->fCarHealth = netgame->incarData->fCarHealth;
		picSync->bytePlayerHealth = netgame->getHealth();
		picSync->bytePlayerArmour = netgame->getArmour();

		uint8_t exKeys = 1;
		picSync->byteCurrentWeapon = (exKeys << 6) | picSync->byteCurrentWeapon & 0x3F;
		picSync->byteCurrentWeapon ^= (picSync->byteCurrentWeapon ^ netgame->bCurrentWeapon) & 0x3F;

		//not support trailer
		picSync->TrailerID_or_ThrustAngle = 0;
	}

	bsVehicleSync.Write((BYTE)ID_VEHICLE_SYNC);
	bsVehicleSync.Write((PCHAR)picSync, sizeof(INCAR_SYNC_DATA));
	netgame->client->Send(&bsVehicleSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

	netgame->dwLastInVehicleDataSentTick = netgame->GetTickCount();
}

void SendPassengerFullSyncData(VEHICLEID vehicleID, CNetGame* netgame)
{
	if (!netgame->vehiclesPool[vehicleID].iDoesExist)
		return;

	if (true)
	{
		RakNet::BitStream bsPassengerSync;

		PASSENGER_SYNC_DATA psSync;
		memset(&psSync, 0, sizeof(PASSENGER_SYNC_DATA));

		psSync.VehicleID = vehicleID;

		psSync.vecPos[0] = netgame->vehiclesPool[vehicleID].fPos[0];
		psSync.vecPos[1] = netgame->vehiclesPool[vehicleID].fPos[1];
		psSync.vecPos[2] = netgame->vehiclesPool[vehicleID].fPos[2];

		psSync.bytePlayerHealth = netgame->fPlayerHealth;//(BYTE)settings.fPlayerHealth;
		psSync.bytePlayerArmour = netgame->fPlayerArmour;//(BYTE)settings.fPlayerArmour;

		psSync.byteDriveBy = 0;
		psSync.byteSeatFlags = netgame->seatFlags;
		psSync.byteCurrentWeapon = 0;
		psSync.lrAnalog = netgame->lrAnalog;
		psSync.udAnalog = netgame->udAnalog;
		psSync.wKeys = netgame->usKeys;

		bsPassengerSync.Write((BYTE)ID_PASSENGER_SYNC);
		bsPassengerSync.Write((PCHAR)&psSync, sizeof(PASSENGER_SYNC_DATA));
		netgame->client->Send(&bsPassengerSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		netgame->dwLastPassengerDataSentTick = GetTickCount();
	}
}

void SendAimSyncData(DWORD dwAmmoInClip, int iReloading, PLAYERID copyFromPlayer, CNetGame* netgame)
{
	if (netgame->dwLastAimDataSentTick < (GetTickCount() - netgame->iNetModeFiringSendRate))
	{
		RakNet::BitStream bsAimSync;
		AIM_SYNC_DATA aimSync;

		if (copyFromPlayer != (PLAYERID)-1)
		{
			if (!netgame->playersInfo[copyFromPlayer].iIsConnected)
				return;

			memcpy((void*)&aimSync, (void*)&netgame->playersInfo[copyFromPlayer].aimData, sizeof(AIM_SYNC_DATA));

			if (aimSync.vecAimPos[0] == 0.0f && aimSync.vecAimPos[1] == 0.0f && aimSync.vecAimPos[2] == 0.0f)
			{
				aimSync.vecAimPos[0] = 0.25f;
			}

			bsAimSync.Write((BYTE)ID_AIM_SYNC);
			bsAimSync.Write((PCHAR)&aimSync, sizeof(AIM_SYNC_DATA));

			netgame->client->Send(&bsAimSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

			netgame->dwLastAimDataSentTick = GetTickCount();
		}
		else
		{
			if (iReloading)
				netgame->playersInfo[netgame->playerID].aimData.byteWeaponState = WS_RELOADING;
			else
				netgame->playersInfo[netgame->playerID].aimData.byteWeaponState = (dwAmmoInClip > 1) ? WS_MORE_BULLETS : dwAmmoInClip;

			netgame->playersInfo[netgame->playerID].aimData.bUnk = 0x55;

			memcpy((void*)&aimSync, (void*)&netgame->playersInfo[netgame->playerID].aimData, sizeof(AIM_SYNC_DATA));

			bsAimSync.Write((BYTE)ID_AIM_SYNC);
			bsAimSync.Write((PCHAR)&aimSync, sizeof(AIM_SYNC_DATA));

			netgame->client->Send(&bsAimSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

			netgame->dwLastAimDataSentTick = GetTickCount();
		}
	}
}

void SendUnoccupiedSyncData(UNOCCUPIED_SYNC_DATA* punocSync, CNetGame* netgame)
{
	if (netgame->dwLastUnocDataSentTick < (GetTickCount() - 30))
	{
		RakNet::BitStream bsUnoccupiedSync;

		bsUnoccupiedSync.Write((BYTE)ID_UNOCCUPIED_SYNC);
		bsUnoccupiedSync.Write((PCHAR)punocSync, sizeof(UNOCCUPIED_SYNC_DATA));
		netgame->client->Send(&bsUnoccupiedSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

		netgame->dwLastUnocDataSentTick = GetTickCount();
	}
}

void SendSpectatorData(SPECTATOR_SYNC_DATA* pSpecData, CNetGame* netgame)
{
	RakNet::BitStream bsSpecSync;

	bsSpecSync.Write((BYTE)ID_SPECTATOR_SYNC);
	bsSpecSync.Write((PCHAR)pSpecData, sizeof(SPECTATOR_SYNC_DATA));

	netgame->client->Send(&bsSpecSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}

void SendBulletData(BULLET_SYNC_DATA* pBulletData, CNetGame* netgame)
{
	RakNet::BitStream bsBulletSync;

	bsBulletSync.Write((BYTE)ID_BULLET_SYNC);
	bsBulletSync.Write((PCHAR)pBulletData, sizeof(BULLET_SYNC_DATA));

	netgame->client->Send(&bsBulletSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}

void SendEnterVehicleNotification(VEHICLEID VehicleID, BOOL bPassenger, CNetGame* netgame)
{
	RakNet::BitStream bsSend;
	BYTE bytePassenger = 0;

	if (bPassenger)
		bytePassenger = 1;

	bsSend.Write(VehicleID);
	bsSend.Write(bytePassenger);
	netgame->client->RPC(&RPC_EnterVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
}

void SendExitVehicleNotification(VEHICLEID VehicleID, CNetGame* netgame)
{
	RakNet::BitStream bsSend;
	bsSend.Write(VehicleID);
	netgame->client->RPC(&RPC_ExitVehicle, &bsSend, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
}

void SendWastedNotification(BYTE byteDeathReason, PLAYERID WhoWasResponsible, CNetGame* netgame)
{
	RakNet::BitStream bsPlayerDeath;

	bsPlayerDeath.Write(byteDeathReason);
	bsPlayerDeath.Write(WhoWasResponsible);
	netgame->client->RPC(&RPC_Death, &bsPlayerDeath, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
}

void NotifyVehicleDeath(VEHICLEID VehicleID, CNetGame* netgame)
{
	RakNet::BitStream bsDeath;
	bsDeath.Write(VehicleID);
	netgame->client->RPC(&RPC_VehicleDestroyed, &bsDeath, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
}

void SendDamageVehicle(WORD vehicleID, DWORD panel, DWORD door, BYTE lights, BYTE tires, CNetGame* netgame)
{
	RakNet::BitStream bsDamageVehicle;

	bsDamageVehicle.Write(vehicleID);
	bsDamageVehicle.Write(panel);
	bsDamageVehicle.Write(door);
	bsDamageVehicle.Write(lights);
	bsDamageVehicle.Write(tires);
	netgame->client->RPC(&RPC_DamageVehicle, &bsDamageVehicle, HIGH_PRIORITY, RELIABLE_ORDERED, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
}
