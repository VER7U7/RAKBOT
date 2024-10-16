/*
	Updated to 0.3.7 by P3ti
*/

// MAIN STUFF /////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#include "raknet/SAMP/SAMP_VER.h"

#define RAKSAMP_VERSION "v0.8.6-0.3.7DL"

#define NETCODE_CONNCOOKIELULZ 0x6969
#define NETGAME_VERSION 4057

#define AUTHOR "jlfm, bartekdvd, P3ti"

#define REJECT_REASON_BAD_VERSION	1
#define REJECT_REASON_BAD_NICKNAME	2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4

#define BP _asm int 3

#define MAX_PLAYERS 1004
#define MAX_VEHICLES 2000
#define MAX_SCRIPTS 32
typedef unsigned short PLAYERID;
typedef unsigned short VEHICLEID;

typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int		int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#define isfinite	_finite

#define EVENT_TYPE_PAINTJOB				1
#define EVENT_TYPE_CARCOMPONENT			2
#define EVENT_TYPE_CARCOLOR				3
#define EVENT_ENTEREXIT_MODSHOP			4

#define BULLET_HIT_TYPE_NONE			0
#define BULLET_HIT_TYPE_PLAYER			1
#define BULLET_HIT_TYPE_VEHICLE			2
#define BULLET_HIT_TYPE_OBJECT			3
#define BULLET_HIT_TYPE_PLAYER_OBJECT   4

#define KEY_UP							-128
#define KEY_DOWN						128
#define KEY_LEFT						-128
#define KEY_RIGHT						128
#define KEY_ACTION						1
#define KEY_CROUCH						2
#define KEY_FIRE						4
#define KEY_SPRINT						8
#define KEY_SECONDARY_ATTACK			16
#define KEY_JUMP						32
#define KEY_LOOK_RIGHT					64
#define KEY_HANDBRAKE					128
#define KEY_LOOK_LEFT					256
#define KEY_SUBMISSION					512
#define KEY_LOOK_BEHIND					512
#define KEY_WALK						1024
#define KEY_ANALOG_UP					2048
#define KEY_ANALOG_DOWN					4096
#define KEY_ANALOG_LEFT					8192
#define KEY_ANALOG_RIGHT				16384

#define MAX_MENU_ITEMS 12
#define MAX_MENU_LINE 32
#define MAX_COLUMNS 2

#define IDB_BUTTON1			10
#define IDB_BUTTON2			11
#define IDE_INPUTEDIT			12
#define IDL_LISTBOX			13

#define DIALOG_STYLE_MSGBOX		0
#define DIALOG_STYLE_INPUT		1
#define DIALOG_STYLE_LIST		2
#define DIALOG_STYLE_PASSWORD		3


//typedef struct _ONFOOT_SYNC_DATA
//{
//	WORD lrAnalog;
//	WORD udAnalog;
//	WORD wKeys;
//	float vecPos[3];
//	float fQuaternion[4];
//	BYTE byteHealth;
//	BYTE byteArmour;
//	BYTE byteCurrentWeapon;
//	BYTE byteSpecialAction;	
//	float vecMoveSpeed[3];
//	float vecSurfOffsets[3];
//	WORD wSurfInfo;
//	int	iCurrentAnimationID;
//} ONFOOT_SYNC_DATA;
#pragma pack(1)
typedef struct _ONFOOT_SYNC_DATA
{
	uint16_t lrAnalog;				// +0
	uint16_t udAnalog;				// +2
	uint16_t wKeys;					// +4
	float vecPos[3];				// +6
	float fQuaternion[4];				// +18
	uint8_t byteHealth;				// +34
	uint8_t byteArmour;				// +35
	uint8_t byteCurrentWeapon;		// +36
	uint8_t byteSpecialAction;		// +37
	float vecMoveSpeed[3];			// +38
	float vecSurfOffsets[3];			// +50
	uint16_t wSurfInfo;				// +62
	uint32_t uiAnimation;			// 64
} ONFOOT_SYNC_DATA;

#pragma pack(1)
typedef struct _INCAR_SYNC_DATA
{
	VEHICLEID VehicleID;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float fQuaternion[4];
	float vecPos[3];
	float vecMoveSpeed[3];
	float fCarHealth;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	BYTE byteCurrentWeapon;
	BYTE byteSirenOn;
	BYTE byteLandingGearState;
	WORD TrailerID_or_ThrustAngle;
	FLOAT fTrainSpeed;
} INCAR_SYNC_DATA;

#pragma pack(1)
typedef struct _PASSENGER_SYNC_DATA
{
	VEHICLEID VehicleID;
	BYTE byteSeatFlags : 7;
	BYTE byteDriveBy : 1;
	BYTE byteCurrentWeapon;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
} PASSENGER_SYNC_DATA;

enum eWeaponState
{
	WS_NO_BULLETS = 0,
	WS_LAST_BULLET = 1,
	WS_MORE_BULLETS = 2,
	WS_RELOADING = 3,
};

#pragma pack(1)
typedef struct _AIM_SYNC_DATA
{
	BYTE byteCamMode;
	float vecAimf1[3];
	float vecAimPos[3];
	float fAimZ;
	BYTE byteCamExtZoom : 6;	// 0-63 normalized
	BYTE byteWeaponState : 2;	// see eWeaponState
	BYTE bUnk;
} AIM_SYNC_DATA;

#pragma pack(1)
typedef struct _UNOCCUPIED_SYNC_DATA // 67
{
	VEHICLEID VehicleID;
	short cvecRoll[3];
	short cvecDirection[3];
	BYTE unk[13];
	float vecPos[3];
	float vecMoveSpeed[3];
	float vecTurnSpeed[3];
	float fHealth;
} UNOCCUPIED_SYNC_DATA;

#pragma pack(1)
typedef struct _SPECTATOR_SYNC_DATA
{
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
} SPECTATOR_SYNC_DATA;

#pragma pack(1)
typedef struct _BULLET_SYNC_DATA {
	BYTE bHitType;
	unsigned short iHitID;
	float fHitOrigin[3];
	float fHitTarget[3];
	float fCenterOfHit[3];
	BYTE bWeaponID;
} BULLET_SYNC_DATA;

#pragma pack(1)
typedef struct _PLAYER_SPAWN_INFO
{
	BYTE byteTeam;
	int iSkin;
	BYTE unk;
	float vecPos[3];
	float fRotation;
	int iSpawnWeapons[3];
	int iSpawnWeaponsAmmo[3];
} PLAYER_SPAWN_INFO;

#pragma pack(1)
typedef struct _PICKUP
{
	int iModel;
	int iType;
	float fX;
	float fY;
	float fZ;
} PICKUP;

#pragma pack(1)
typedef struct _TEXT_DRAW_TRANSMIT
{
	union
	{
		BYTE byteFlags;
		struct
		{
			BYTE byteBox : 1;
			BYTE byteLeft : 1;
			BYTE byteRight : 1;
			BYTE byteCenter : 1;
			BYTE byteProportional : 1;
			BYTE bytePadding : 3;
		};
	};
	float fLetterWidth;
	float fLetterHeight;
	DWORD dwLetterColor;
	float fLineWidth;
	float fLineHeight;
	DWORD dwBoxColor;
	BYTE byteShadow;
	BYTE byteOutline;
	DWORD dwBackgroundColor;
	BYTE byteStyle;
	BYTE byteSelectable;
	float fX;
	float fY;
	WORD wModelID;
	float fRotX;
	float fRotY;
	float fRotZ;
	float fZoom;
	WORD wColor1;
	WORD wColor2;
} TEXT_DRAW_TRANSMIT;

#pragma pack(1)
typedef struct _NEW_VEHICLE {
    VEHICLEID VehicleId;
	int		  iVehicleType;
	float	  vecPos[3];
	float	  fRotation;
	BYTE	  aColor1;
	BYTE	  aColor2;
	float	  fHealth;
	BYTE	  byteInterior;
	DWORD	  dwDoorDamageStatus;
	DWORD	  dwPanelDamageStatus;
	BYTE	  byteLightDamageStatus;
	BYTE	  byteTireDamageStatus;
	BYTE	  byteAddSiren;
	BYTE      byteModSlots[14];
	BYTE	  bytePaintjob;
	DWORD	  cColor1;
	DWORD	  cColor2;
	BYTE	  byteUnk;
} NEW_VEHICLE;

#pragma pack(1)
struct stPlayerInfo
{
	int iIsConnected;
	int iIsStreamedIn;
	int iGotMarkersPos;
	char szPlayerName[20];
	int iScore;
	DWORD dwPing;
	int iAreWeInAVehicle;
	BYTE byteInteriorId;
	BYTE byteIsNPC;

	// STORED INFO
	ONFOOT_SYNC_DATA onfootData;
	INCAR_SYNC_DATA incarData;
	PASSENGER_SYNC_DATA passengerData;
	AIM_SYNC_DATA aimData;
	UNOCCUPIED_SYNC_DATA unocData;
	BULLET_SYNC_DATA bulletData;
};

struct stVehiclePool
{
	int iDoesExist;
	float fPos[3];
	int iModelID;
};

struct MENU_INT
{
	BOOL bMenu;
	BOOL bRow[MAX_MENU_ITEMS];
	BOOL bPadding[8 - ((MAX_MENU_ITEMS + 1) % 8)];
};
struct stGTAMenu
{
	char szTitle[MAX_MENU_LINE];
	char szSeparator[MAX_MENU_LINE]; // ???
	BYTE byteColCount;
	char szColumnContent[MAX_COLUMNS][MAX_MENU_LINE];
};

struct stSAMPDialog
{
	int iIsActive;
	BYTE bDialogStyle;
	WORD wDialogID;
	BYTE bTitleLength;
	char szTitle[257];
	BYTE bButton1Len;
	char szButton1[257];
	BYTE bButton2Len;
	char szButton2[257];
	char szInfo[257];
};

// GTA STUFF //////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#define VEHICLE_LIST_ID_START			400
#define VEHICLE_LIST_SIZE				212

#define VEHICLE_CLASS_CAR				0
#define VEHICLE_CLASS_CAR_FAST			1
#define VEHICLE_CLASS_HEAVY				2
#define VEHICLE_CLASS_HELI				3
#define VEHICLE_CLASS_AIRPLANE			4
#define VEHICLE_CLASS_BIKE				5
#define VEHICLE_CLASS_BOAT				6
#define VEHICLE_CLASS_MINI				7
#define VEHICLE_CLASS_TRAILER			8
#define VEHICLE_CLASS_COUNT				9	/* # of classes */

struct vehicle_entry
{
	int			id;			// model id
	int			class_id;	// class id
	const char	*name;		// vehicle name
	int			passengers; // total passenger seats, 0-10
};

extern struct vehicle_entry vehicle_list[VEHICLE_LIST_SIZE];