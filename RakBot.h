#pragma once

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <cmath>

#include "Math.h"
#include "Log.h"
#include "gettimeofday.h"
#include "common.h"
#include "math_stuff.h"

#define RAKSAMP_CLIENT

#include "raknet/RakClientInterface.h"
#include "raknet/RakNetworkFactory.h"
#include "raknet/BitStream.h"
#include "raknet/PacketEnumerations.h"
#include "raknet/SAMP/SAMPRPC.h"
#include "raknet/SAMP/samp_netencr.h"
#include "raknet/SAMP/samp_auth.h"


//CNET
#include "CNetGame.h"
#include "CNetRPC.h"
#include "SampFuncs.h"

//Game
#include "Game/GLocalPlayer.h"