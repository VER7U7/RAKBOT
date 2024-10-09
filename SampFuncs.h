#include "RakBot.h"
#include "Game/GLocalPlayer.h"

class CNetGame;

void onFootUpdateAtNormalPos(CNetGame* netgame);
void onFootUpdateFollow(PLAYERID followID, CNetGame* netgame);
void inCarUpdateFollow(PLAYERID followID, VEHICLEID withVehicleID, CNetGame* netgame);
void spectatorUpdate(CNetGame* netgame);

//int sampConnect(char* szHostname, int iPort, char* szNickname, char* szPassword, RakClientInterface* pRakClient);
//void sampDisconnect(int iTimeout);
void sampRequestClass(int iClass, CNetGame* netgame);
void sampSpawn(CNetGame* netgame);
void sampSpam(CNetGame* netgame);
void sampFakeKill();
void sampLag();
void sampJoinFlood();
void sampChatFlood();
void sampClassFlood();
void sendServerCommand(char* szCommand, CNetGame* netgame);
void sendChat(char* szMessage, CNetGame* netgame);

void sendScmEvent(int iEventType, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, CNetGame* netgame);
void sendDialogResponse(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem, char* szInputResp, CNetGame* netgame);
void sendPickUp(int iPickupID, CNetGame* netgame);
void selectTextDraw(int iTextDrawID, CNetGame* netgame);

int isPlayerConnected(PLAYERID iPlayerID, CNetGame* netgame);
int getPlayerID(char* szPlayerName, CNetGame* netgame);
char* getPlayerName(PLAYERID iPlayerID, CNetGame* netgame);
int getPlayerPos(PLAYERID iPlayerID, float* fPos, CNetGame* netgame);

PLAYERID getPlayerIDFromPlayerName(char* szName, CNetGame* netgame);
unsigned short getPlayerCount(CNetGame* netgame);
unsigned short getPlayerCountWoNPC(CNetGame* netgame);


void SetStringFromCommandLine(char* szCmdLine, char* szString);

void ApplyAnimation(CNetGame* netgame);
void ClearAnimation(CNetGame* netgame);


extern const char g_szAnimBlockNames[1843][40];

const char* GetAnimByIdx(int idx);
int GetAnimIdxByName(const char* szName);