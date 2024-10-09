#pragma once
#include <cmath>
#include <string>

struct VELOCITY {
	float x;
	float y;
	float z;
};

struct POS {
	float x;
	float y;
	float z;
	POS() {
		x = 0;
		y = 0;
		z = 0;
	}
	POS(float x1, float y1, float z1) {
		x = x1;
		y = y1;
		z = z1;
	}
};

struct QUATERNION {
	float x;
	float y;
	float z;
	float w;
};

VELOCITY CalcVelocity(POS myPos, POS targetPos, VELOCITY speed, POS accuracy);

float PosToRotate(POS a, POS b);

QUATERNION eulerToQuaternion(float roll, float pitch, float yaw);

POS toPOS(float x, float y, float z);

QUATERNION toQuaternion(float x, float y, float z, float w);

VELOCITY toVelocity(float x, float y, float z);

int getRandomNumber(int min, int max);

std::string gen_random(const int len);