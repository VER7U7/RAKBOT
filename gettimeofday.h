
#include "RakBot.h"
#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

// MSVC defines this in winsock2.h!?
//typedef struct timeval {
//    long tv_sec;
//    long tv_usec;
//} timeval;

uint32_t GetTicksCount();

int gettimeday(struct timeval* tp, struct timezone* tzp);

#else
#include <sys/time.h>

int gettimeday(struct timeval* tp, struct timezone* tzp)
{
    return gettimeofday(tp, tzp);
}

#endif