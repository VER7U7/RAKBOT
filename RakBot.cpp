#include "RakBot.h"

using namespace std;

int init = 0;

bool login = false;

void CustomFunc(CNetGame* netgame) {
	SPECTATOR_SYNC_DATA specSync;
	specSync.wKeys = KEY_FIRE;

	/*if (netgame->bIsSpectating) {
		SendSpectatorData(&specSync, netgame);
		sampRequestClass(1, netgame);
		sampSpawn(netgame);
	}

	if (!init) {
		sampRequestClass(1, netgame);
		sampSpawn(netgame);
		init = true;
	}*/

	if (netgame->sampDialog.iIsActive) {
		srand(time(0));
		if (!login) {
			if (netgame->sampDialog.wDialogID == 1) {
				sendDialogResponse(1, 1, 0, "123123", netgame);
				netgame->sampDialog.iIsActive = false;
			}else 
			if (netgame->sampDialog.wDialogID == 2) {
				char bombom[23];
				strcpy(bombom, (gen_random(12) + "@gmail.com").c_str());
				Log(bombom);
				sendDialogResponse(2, 1, 0, bombom, netgame);
				netgame->sampDialog.iIsActive = false;
			}else
			if (netgame->sampDialog.wDialogID == 3) {
				sendDialogResponse(3, 1, 0, "", netgame);
				netgame->sampDialog.iIsActive = false;
			}
			else {
				sendDialogResponse(netgame->sampDialog.wDialogID, 1, 0, "", netgame);
				netgame->sampDialog.iIsActive = false;
			}
		}
		else {
			if (netgame->sampDialog.wDialogID == 4) {
				sendDialogResponse(4, 1, 0, "123123", netgame);
				netgame->sampDialog.iIsActive = false;
			}
		}
	}
	


	if (netgame->bStart) {
		/*if (netgame->inVehicle == false) {
			netgame->EnterVehicle(false, 1028, 0);
		}*/
		
	}
}

class Test {
public: 
	static void TestVoid(CNetGame* netgame, RPCParameters* rpcParams) {
		Test* thiz = (Test*)netgame->ptrToClass;
		thiz->Print();
	}

	void Print() {
		std::cout << "Print" << std::endl;
	}
};

static void OnSendClientMessage(CNetGame* netgame, RPCParameters* rpcParams) {
	if (netgame->vClientMessages.size() > 0) {
		std::string str = netgame->vClientMessages.at(netgame->vClientMessages.size() - 1);
		Log("Chat: " + str);
	}
}

static void OnRequestSpawn(CNetGame* netgame, RPCParameters* rpcParams) {
	if (netgame->bSpawnRequest) {
		sampRequestClass(0, netgame);
		netgame->iSpawned = false;
	}
}

int main()
{
	srand(time(0));
	CNetGame* netGame = new CNetGame("192.168.0.104", 7777, "TEST_BOT", nullptr, AUTH_DEFAULT);
	netGame->SetCustomProcess(true, CustomFunc);
	netGame->sendRequestClass = false;
	netGame->SetCustomRPC(CUSTOM_ClientMessage, OnSendClientMessage);
	netGame->SetCustomRPC(CUSTOM_ScrTogglePlayerSpectating, OnRequestSpawn);

	cout << "Started" << endl;
	while (true) {

		Sleep(20000);
		sampRequestClass(0, netGame);
		sampSpawn(netGame);
		std::cout << netGame->getPos().x << " " << netGame->getPos().y << " " << netGame->getPos().z << std::endl;
	}

	return 0;
}
