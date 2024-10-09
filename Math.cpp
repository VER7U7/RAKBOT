#include "Math.h"


VELOCITY CalcVelocity(POS myPos, POS targetPos, VELOCITY speed, POS accuracy) {
	VELOCITY velocity;
	velocity.x = 0; velocity.y = 0; velocity.z = 0;

	float dX = targetPos.x - myPos.x;
	float dY = targetPos.y - myPos.y;
	float dZ = targetPos.z - myPos.z;

	float timeX = 0, timeY = 0, timeZ = 0;

	if (abs(dX) > accuracy.x) {
		timeX = abs(dX) / speed.x;
		velocity.x = dX / timeX;
	}
	if (abs(dY) > accuracy.y) {
		timeY = abs(dY) / speed.y;
		velocity.y = dY / timeY;
	}
	if (abs(dZ) > accuracy.z) {
		timeZ = abs(dZ) / speed.z;
		velocity.z = dZ / timeZ;
	}

	return velocity;
}

float PosToRotate(POS a, POS b) {
	float startRot[3] = { 0,0,0 };
	float defference[3];
	defference[0] = (b.x - a.x);
	defference[1] = (b.y - a.y);
	defference[2] = (b.z - a.z);
	float angle_radian = atan2(defference[0], defference[1]);
	return angle_radian;
}

QUATERNION eulerToQuaternion(float roll, float pitch, float yaw) {
	QUATERNION q;
	float cy = cos(yaw * 0.5);
	float sy = sin(yaw * 0.5);
	float cp = cos(pitch * 0.5);
	float sp = sin(pitch * 0.5);
	float cr = cos(roll * 0.5);
	float sr = sin(roll * 0.5);

	q.x = cy * cp * cr + sy * sp * sr;
	q.w = cy * cp * sr - sy * sp * cr;
	q.y = sy * cp * sr + cy * sp * cr;
	q.z = sy * cp * cr - cy * sp * sr;

	return q;
}

POS toPOS(float x, float y, float z) {
	POS pos;
	pos.x = x; pos.y = y; pos.z = z;
	return pos;
}

QUATERNION toQuaternion(float x, float y, float z, float w) {
	QUATERNION quat;
	quat.x = x; quat.y = y; quat.z = z; quat.w = w;
	return quat;
}

VELOCITY toVelocity(float x, float y, float z) {
	VELOCITY vel;
	vel.x = x; vel.y = y; vel.z = z;
	return vel;
}

int getRandomNumber(int min, int max) {
	static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
	// Равномерно распределяем рандомное число в нашем диапазоне
	return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

std::string gen_random(const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}