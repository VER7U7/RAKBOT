#pragma once

#include "RakBot.h"
#include <unordered_map>

class CNetGame;

class CNetRPC {
public:
	//RPC
	static void ServerJoin(RPCParameters* rpcParams);
	static void ServerQuit(RPCParameters* rpcParams);
	static void InitGame(RPCParameters* rpcParams);
	static void WorldPlayerAdd(RPCParameters* rpcParams);
	static void WorldPlayerDeath(RPCParameters* rpcParams);
	static void WorldPlayerRemove(RPCParameters* rpcParams);
	static void WorldVehicleAdd(RPCParameters* rpcParams);
	static void WorldVehicleRemove(RPCParameters* rpcParams);
	static void ConnectionRejected(RPCParameters* rpcParams);
	static void ClientMessage(RPCParameters* rpcParams);
	static void Chat(RPCParameters* rpcParams);
	static void UpdateScoresPingsIPs(RPCParameters* rpcParams);
	static void SetCheckpoint(RPCParameters* rpcParams);
	static void DisableCheckpoint(RPCParameters* rpcParams);
	static void Pickup(RPCParameters* rpcParams);
	static void DestroyPickup(RPCParameters* rpcParams);
	static void RequestClass(RPCParameters* rpcParams);
	static void ScrInitMenu(RPCParameters* rpcParams);
	static void ScrDialogBox(RPCParameters* rpcParams);
	static void ScrGameText(RPCParameters* rpcParams);
	static void ScrPlayAudioStream(RPCParameters* rpcParams);
	static void ScrSetDrunkLevel(RPCParameters* rpcParams);
	static void ScrHaveSomeMoney(RPCParameters* rpcParams);
	static void ScrResetMoney(RPCParameters* rpcParams);
	static void ScrSetPlayerPos(RPCParameters* rpcParams);
	static void ScrSetPlayerFacingAngle(RPCParameters* rpcParams);
	static void ScrSetSpawnInfo(RPCParameters* rpcParams);
	static void ScrSetPlayerHealth(RPCParameters* rpcParams);
	static void ScrSetPlayerArmour(RPCParameters* rpcParams);
	static void ScrSetPlayerSkin(RPCParameters* rpcParams);
	static void ScrCreateObject(RPCParameters* rpcParams);
	static void ScrCreate3DTextLabel(RPCParameters* rpcParams);
	static void ScrShowTextDraw(RPCParameters* rpcParams);
	static void ScrHideTextDraw(RPCParameters* rpcParams);
	static void ScrEditTextDraw(RPCParameters* rpcParams);
	static void ScrTogglePlayerSpectating(RPCParameters* rpcParams);
	static void GiveTakeDamage(RPCParameters* rpcParams);
	static void RequestSpawn(RPCParameters* rpcParams);
	static void EnterVehicle(RPCParameters* rpcParams);
	static void ExitVehicle(RPCParameters* rpcParams);


	void RegisterRPC(RakClientInterface* client);
	void UnRegisterRPC(RakClientInterface* client);
	CNetRPC(CNetGame* netgame, int instanceID);
	~CNetRPC() { instanceID = 0; };
private:
	
public: 
	int instanceID;
	CNetGame* netgame;
	static std::unordered_map<int, CNetRPC*> rpcInstances;
};