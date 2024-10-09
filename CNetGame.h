#pragma once

#include <stdint.h>

#include "RakBot.h"

#define GAMESTATE_WAIT_CONNECT	9
#define GAMESTATE_CONNECTING	13
#define GAMESTATE_AWAIT_JOIN	15
#define GAMESTATE_CONNECTED 	14 
#define GAMESTATE_RESTARTING	18
#define GAMESTATE_NONE 			0
#define GAMESTATE_DISCONNECTED	4

#define NETMODE_ONFOOT_SENDRATE	30
#define NETMODE_INCAR_SENDRATE	30
#define NETMODE_FIRING_SENDRATE 30
#define NETMODE_SEND_MULTIPLIER 2

#define INVALID_PLAYER_ID 0xFFFF
#define INVALID_VEHICLE_ID 0xFFFF

#define TICKTIME(x) (x * 60)

//ANIM FLAGS
#define ANIMFLAG_TO_TASKANIM 1
#define ANIMFLAG_NONAME_2 2
#define ANIMFLAG_NONAME_4 4
#define ANIMFLAG_NONAME_8 8
#define ANIMFLAG_NONAME_64 64
#define ANIMFLAG_NONAME_2048 2048
#define ANIMFLAG_NONAME_4096 4096
#define ANIMFLAG_NONAME_16384  16384 
#define ANIMFLAG_NONAME_32768  32768  
#define ANIMFLAG_NONAME_65536  65536   
#define ANIMFLAG_NONAME_131072  131072    
#define ANIMFLAG_UPPER_BODY 16
#define ANIMFLAG_NORMAL 19
#define ANIMFLAG_FOLLOW_MOVEMENT 32
#define ANIMFLAG_STOP 128
#define ANIMFLAG_INVERT 256 // UNKNOW #1
#define ANIMFLAG_SURF 512 //UNKNOW
#define ANIMFLAG_INVERT2 8192  //UNKNOW #2
#define ANIMFLAG_SOME_SCENARIOS 262144   //UNKNOW


enum AUTHTYPES {
	AUTH_FFFF,
	AUTH_ARIZONA,
	AUTH_ARIZONAMOBILE,

	AUTH_DEFAULT
};

enum CUSTOM_RPC {
	CUSTOM_ServerJoin,
	CUSTOM_ServerQuit,
	CUSTOM_InitGame,
	CUSTOM_WorldPlayerAdd,
	CUSTOM_WorldPlayerDeath,
	CUSTOM_WorldPlayerRemove,
	CUSTOM_WorldVehicleAdd,
	CUSTOM_WorldVehicleRemove,
	CUSTOM_ConnectionRejected,
	CUSTOM_ClientMessage,
	CUSTOM_Chat,
	CUSTOM_UpdateScoresPingsIPs,
	CUSTOM_SetCheckpoint,
	CUSTOM_DisableCheckpoint,
	CUSTOM_Pickup,
	CUSTOM_DestroyPickup,
	CUSTOM_RequestClass,
	CUSTOM_ScrInitMenu,
	CUSTOM_ScrDialogBox,
	CUSTOM_ScrGameText,
	CUSTOM_ScrPlayAudioStream,
	CUSTOM_ScrSetDrunkLevel,
	CUSTOM_ScrHaveSomeMoney,
	CUSTOM_ScrResetMoney,
	CUSTOM_ScrSetPlayerPos,
	CUSTOM_ScrSetFacingAngle,
	CUSTOM_ScrSetSpawnInfo,
	CUSTOM_ScrSetPlayerHealth,
	CUSTOM_ScrSetPlayerArmour,
	CUSTOM_ScrSetPlayerSkin,
	CUSTOM_ScrCreateObject,
	CUSTOM_ScrCreate3DTextLabel,
	CUSTOM_ScrShowTextDraw,
	CUSTOM_ScrHideTextDraw,
	CUSTOM_ScrEditTextDraw,
	CUSTOM_ScrTogglePlayerSpectating,
	CUSTOM_GiveTakeDamage,
	CUSTOM_RequestSpawn,
	CUSTOM_EnterVehicle,
	CUSTOM_ExitVehicle
};

enum WEAPON {
	WEAPON_FIST,
	WEAPON_BRASSKNUCKLE,
	WEAPON_GOLFCLUB,
	WEAPON_NITESTICK,
	WEAPON_KNIFE,
	WEAPON_BAT,
	WEAPON_SHOVEL,
	WEAPON_POOLSTICK,
	WEAPON_KATANA,
	WEAPON_CHAINSAW,
	WEAPON_DILDO,
	WEAPON_DILDO2,
	WEAPON_VIBRATOR,
	WEAPON_VIBRATOR2,
	WEAPON_FLOWER,
	WEAPON_CANE,
	WEAPON_GRENADE,
	WEAPON_TEARGAS,
	WEAPON_MOLOTOV,
	WEAPON_COLT45 = 22,
	WEAPON_SILENCED,
	WEAPON_DEAGLE,
	WEAPON_SHOTGUN,
	WEAPON_SAWEDOFF,
	WEAPON_SHOTGSPA,
	WEAPON_UZI,
	WEAPON_MP5,
	WEAPON_AK47,
	WEAPON_M4,
	WEAPON_TEC9,
	WEAPON_RIFLE,
	WEAPON_SNIPER,
	WEAPON_ROCKETLAUNCHER,
	WEAPON_HEATSEEKER,
	WEAPON_FLAMETHROWER,
	WEAPON_MINIGUN,
	WEAPON_SATCHEL,
	WEAPON_BOMB,
	WEAPON_SPRAYCAN,
	WEAPON_FIREEXTINGUISHER,
	WEAPON_CAMERA,
	WEAPON_NIGHTVISION,
	WEAPON_THERMALGOGGLES,
	WEAPON_PARACHUTE,
	WEAPON_VEHICLE = 49,
	WEAPON_HELICOPTERBLADES,
	WEAPON_EXPLOSION,
	WEAPON_DROWN,
	WEAPON_COLLISION,
	WEAPON_CONNECT = 200,
	WEAPON_DISCONNECT,
	WEAPON_SUICIDE = 255
};

struct PICKUP_PACK {
	int PickupID;
	PICKUP Pickup;
};

class CNetRPC;

uint32_t GetAnimationIndexFlag(int AnimID, uint32_t flag, uint32_t blend);
uint32_t GetAnimationIndexFlag(int AnimID);
uint16_t DecodeAnimation(uint32_t encAnim, uint32_t* outBlend, uint32_t* outFlags);
uint16_t DecodeAnimation(uint32_t encAnim);

class CNetGame {
public:
	CNetGame(const char* szHostOrIp, int iPort, const char* szPlayerName, const char* szPass, int AUTH_TYPE);
	~CNetGame();

	void Process();

	//Getters
	//Get the state of the game
	int GetGameState() { return GameState; }
	uint32_t GetTickCount() { return this->ticksCount; }
	uint32_t GetNewThisID();
	//Getting the internal ID of the called class (when calling a new same class, it will be different)
	uint32_t GetThisID() { return ThisID; }

	//Reseters
	void resetPools(int iRestart);
	void RemoveThisID();
	//Removes installed Custom RPCs under the passed number in customID
	bool DestroyCustomRPC(int customID);

	//Senders


	//Setters
	//Sets the state of the game
	void SetGameState(int state) { GameState = state; }

	//A link to a function that will be called 1000 / 30 per second is set.
	//If the connected parameter is set to true 
	//then it is called when you have already connected to the server, 
	//if false then vice versa.
	void SetCustomProcess(bool connected, void (*functionPointer)(CNetGame* netgame)) {
			if (connected)
				CustomProcess = functionPointer;
			else
				CustomProcessNC = functionPointer;
		}
	//The function reference is called every time it is received from the RPC server whose id is specified in customID.
	//You can see all supported id in enum CUSTOM_RPC
	bool SetCustomRPC(int customID, void (*CustomRPC)(CNetGame* netgame, RPCParameters* rpcParams));

	//Updaters
	void UpdateNetwork();
	void UpdatePlayerScoresAndPings(int iWait, int iMS, RakClientInterface* pRakClient);

	//Packets
	void Packet_AuthKey(Packet* p);
	void Packet_ConnectionSucceeded(Packet* p);
	void Packet_PlayerSync(Packet* p);
	void Packet_VehicleSync(Packet* p);
	void Packet_PassengerSync(Packet* p);
	void Packet_MarkersSync(Packet* p);
	void Packet_AimSync(Packet* p);
	void Packet_BulletSync(Packet* pkt);
	void Packet_TrailerSync(Packet* p);
	void Packet_CustomRPC(Packet* p);
	void Packet_DisconnectionNotification(Packet* p);
	void Packet_ConnectionLost(Packet* p);

public: //Player Control
	//Move the character to the specified point. 'speed' is used to set the speed for each axis.
	//'animID' sets the animation while moving, according to the 1257 standard
	//WARNING : Do not use in main threads(such as SetCustomProcess or MainProcess in CNetGame)
	bool GoTo(POS target, VELOCITY speed, int animID = 1257);

	//We move the car(in which we are) to the specified point. "Speed" is used to set the speed for each axis.
	//WARNING: do not use in main threads(such as SetCustomProcess or MainProcess in CNetGame)
	bool GoToVehicle(POS target, VELOCITY speed);

	//The function call puts the player in the car and notifies the server about it.
	//'seatFlags' - indicates where the character will sit.
	void EnterVehicle(bool bPassenger, int iVehicleID, unsigned char seatFlags);

	//The function call exits the character from the car
	void ExitVehicle(int iVehicleID);


private:
	
	void (*CustomProcess)(CNetGame* netgame) = nullptr;
	void (*CustomProcessNC)(CNetGame* netgame) = nullptr;

	//Custom RPC Variables
	void (*CustomServerJoin)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomServerQuit)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomInitGame)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomWorldPlayerAdd)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomWorldPlayerDeath)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomWorldPlayerRemove)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomWorldVehicleAdd)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomWorldVehicleRemove)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomConnectionRejected)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomClientMessage)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomChat)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomUpdateScoresPingsIPs)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomSetCheckpoint)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomDisableCheckpoint)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomPickup)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomDestroyPickup)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomRequestClass)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrInitMenu)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrDialogBox)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrGameText)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrPlayAudioStream)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetDrunkLevel)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrHaveSomeMoney)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrResetMoney)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetPlayerPos)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetFacingAngle)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetSpawnInfo)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetPlayerHealth)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetPlayerArmour)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrSetPlayerSkin)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrCreateObject)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrCreate3DTextLabel)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrShowTextDraw)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrHideTextDraw)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrEditTextDraw)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomScrTogglePlayerSpectating)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomGiveTakeDamage)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomRequestSpawn)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomEnterVehicle)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	void (*CustomExitVehicle)(CNetGame* netgame, RPCParameters* rpcParams) = nullptr;
	
	uint32_t Ptime = 0;
	static std::vector<uint32_t> allThisID;

	//Packets methods
	void MainProcess();
	void InitVars();

	//private methods
	void StartClient() { MainProcessActive = true; }
	void StopClient() { MainProcessActive = false; }

private:
	bool bInfinityHealth;
	bool bInfinityArmour;
	public:
	bool bSpawnRequest = false;

public:
	//ticks
	uint32_t dwLastDisconnection = GetTickCount();
	uint32_t dwLastFakeKill = GetTickCount();
	uint32_t dwLastLag = GetTickCount();
	uint32_t dwLastJoinFlood = GetTickCount();
	uint32_t dwLastChatFlood = GetTickCount();
	uint32_t dwLastClassFlood = GetTickCount();
	uint32_t dwLastBulletFlood = GetTickCount();
	uint32_t inCarUpdateTick = GetTickCount();
	uint32_t uiLastInfoUpdate = GetTickCount();
	uint32_t uiLastStatsUpdate = GetTickCount();
	uint32_t dwLastOnFootDataSentTick = GetTickCount();
	uint32_t dwLastInVehicleDataSentTick = GetTickCount();
	uint32_t dwLastPassengerDataSentTick = GetTickCount();
	uint32_t dwLastAimDataSentTick = GetTickCount();
	uint32_t dwLastUnocDataSentTick = GetTickCount();
	uint32_t uiLastCustomProcessTick = GetTickCount();

public: //local vars
	uint32_t lastConnectTime;
	bool MainProcessActive = false;
	int GameState;
	CNetRPC* rpcFactory;
	RakClientInterface* client;
	uint32_t ThisID = 0;

public: //Variables
	uint32_t ticksCount = 0;
	char m_szHostOrIp[0x7F];
	char m_szHostName[0xFF];
	int m_iPort;
	char m_szPlayerName[0xFF];
	PLAYERID playerID;
	char m_szClientVersion[0xFF];
	int m_iAuthType;
	char AUTH_BS[0xFF];

	bool sendRequestClass = true;
	
public: //global world variables
	bool m_bZoneNames = false, m_bUseCJWalk = false, m_bAllowWeapons, m_bLimitGlobalChatRadius;
	float m_fGlobalChatRadius, m_fNameTagDrawDistance = 60.f;
	bool m_bDisableEnterExits = false, m_bNameTagLOS = false, m_bManualVehicleEngineAndLight;
	bool m_bShowPlayerTags;
	int m_iShowPlayerMarkers;
	BYTE m_byteWorldTime=12, m_byteWeather=10;
	float m_fGravity = (float)0.008000000;
	int m_iDeathDropMoney=0;
	bool m_bInstagib=false;

	int m_iSpawnsAvailable=0;
	int m_iLagCompensation;
	int m_iVehicleFrendlyFire;

	int iNetModeNormalOnfootSendRate = NETMODE_ONFOOT_SENDRATE;
	int iNetModeNormalInCarSendRate = NETMODE_INCAR_SENDRATE;
	int iNetModeFiringSendRate = NETMODE_FIRING_SENDRATE;
	int iNetModeSendMultiplier = NETMODE_SEND_MULTIPLIER;

	float fNormalModeRot = 0;

public: //CheckPoints
	float fCurrentCheckpoint[3];
	float fSizeCheckpoint = 0.f;
	bool bActiveCheckpoint = false;

public: //pickups
	std::vector<PICKUP_PACK> vPickupInfo;

public: //player variables
	int iLastMoney=0, iLastDrunkLevel=0;
	int iDrunkLevel=0, iMoney=0, iLocalPlayerSkin=0;
	int iFollowingPassenger=0, iFollowingDriver=0;
	int iPassengerNotificationSent = 0, iDriverNotificationSent = 0;
	int iSpawned=0;
	bool bIsSpectating=0;
	PLAYER_SPAWN_INFO SpawnInfo;

	BYTE bCurrentWeapon=0;
	float fPlayerHealth=100;
		float getHealth() { return fPlayerHealth; }
		void setHealth(float health) { fPlayerHealth = health; }
	float fPlayerArmour=0;
		float getArmour() { return fPlayerArmour; }
		void setArmour(float armour) { fPlayerArmour = armour; }

	uint32_t uiAnimation;
		uint32_t getAnimation() { return uiAnimation; }
		void setAnimation(uint32_t anim) { uiAnimation = anim; }

	float fNormalModePos[3];
		POS getPos() { return toPOS(fNormalModePos[0], fNormalModePos[1], fNormalModePos[2]); }
		void setPos(POS pos) { fNormalModePos[0] = pos.x; fNormalModePos[1] = pos.y; fNormalModePos[2] = pos.z; };
	
	float vecMoveSpeed[3];
		VELOCITY getMoveSpeed() { return toVelocity(vecMoveSpeed[0], vecMoveSpeed[1], vecMoveSpeed[2]); }
		void setMoveSpeed(VELOCITY vel) { vecMoveSpeed[0] = vel.x; vecMoveSpeed[1] = vel.y; vecMoveSpeed[2] = vel.z; }
	
	float fQuaternion[4];
		QUATERNION getQuaternion() { return toQuaternion(fQuaternion[0], fQuaternion[1], fQuaternion[2], fQuaternion[3]); }
		void setQuaternion(QUATERNION quat) { fQuaternion[0] = quat.x; fQuaternion[1] = quat.y; fQuaternion[2] = quat.z; fQuaternion[3] = quat.w; }
	
	float vecSurfOffsets[3];
		POS getSurfOffsets() { return toPOS(vecSurfOffsets[0], vecSurfOffsets[1], vecSurfOffsets[2]); }
		void setSurfOffsets(POS surf) { vecSurfOffsets[0] = surf.x; vecSurfOffsets[1] = surf.y; vecSurfOffsets[2] = surf.z; }
	
	uint16_t usSurfInfo;
		uint16_t getSurfInfo() { return usSurfInfo; }
		void setSurfInfo(uint16_t surfInfo) { usSurfInfo = surfInfo; }
	
	uint8_t byteSpecialAction;
		uint8_t getSpecialAction() { return byteSpecialAction; }
		void setSpecialAction(uint8_t specAction) { byteSpecialAction = specAction; }
	
	uint16_t lrAnalog;
		uint16_t getLR() { return lrAnalog; }
		void setLR(uint16_t lr) { lrAnalog = lr; }

	uint16_t udAnalog;
		uint16_t getUD() { return udAnalog; }
		void setUD(uint16_t ud) { udAnalog = ud; }

	uint16_t usKeys;
		uint16_t getKeys() { return usKeys; }
		void setKeys(uint16_t keys) { usKeys = keys; }

	stGTAMenu GTAMenu;
	stSAMPDialog sampDialog;
	
public: //in vehicle variables
	bool inVehicle = false;
	bool isPassenger = false;
	int inVehicleID = 0;
	stVehiclePool sInVehicle;
	unsigned char seatFlags = 1;
	INCAR_SYNC_DATA* incarData = nullptr;
	std::vector<int> PassengerID;

public: 
	//ChatMessages
	std::vector<std::string> vChatMessages;
	std::vector<std::string> vClientMessages;

public: //global players vars
	stPlayerInfo playersInfo[MAX_PLAYERS];
    stVehiclePool vehiclesPool[MAX_VEHICLES];
    NEW_VEHICLE fullVehiclesPool[MAX_VEHICLES];

public: // friend classes
	friend class CNetRPC;

public:
	//Setting a reference to the parent class (Can be used in custom rpc)
	void* ptrToClass = nullptr;

public: //test vars
	bool bStart;
};