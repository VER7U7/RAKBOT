#include "../RakBot.h"

class CNetGame;

void SendOnFootFullSyncData(ONFOOT_SYNC_DATA* pofSync, int sendDeathNoti, PLAYERID followPlayerID, CNetGame* netgame);
void SendInCarFullSyncData(INCAR_SYNC_DATA* picSync, int iVehicleID, CNetGame* netgame);
void SendPassengerFullSyncData(VEHICLEID vehicleID, CNetGame* netgame);
void SendAimSyncData(DWORD dwAmmoInClip, int iReloading, PLAYERID copyFromPlayer, CNetGame* netgame);
void SendUnoccupiedSyncData(UNOCCUPIED_SYNC_DATA* punocSync, CNetGame* netgame);
void SendSpectatorData(SPECTATOR_SYNC_DATA* pSpecData, CNetGame* netgame);
void SendBulletData(BULLET_SYNC_DATA* pBulletData, CNetGame* netgame);

void SendEnterVehicleNotification(VEHICLEID VehicleID, BOOL bPassenger, CNetGame* netgame);
void SendExitVehicleNotification(VEHICLEID VehicleID, CNetGame* netgame);
void SendWastedNotification(BYTE byteDeathReason, PLAYERID WhoWasResponsible, CNetGame* netgame);
void NotifyVehicleDeath(VEHICLEID VehicleID, CNetGame* netgame);
void SendDamageVehicle(WORD vehicleID, DWORD panel, DWORD door, BYTE lights, BYTE tires, CNetGame* netgame);