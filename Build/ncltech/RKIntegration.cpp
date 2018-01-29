#include "RKIntegration.h"



RKIntegration::RKIntegration()
{
}


RKIntegration::~RKIntegration()
{
}


Vector3 RKIntegration::integVelocity(Vector3 v, Vector3 a, float dt)
{
	Vector3 k1 = v + a*dt;

	Vector3 k2 = v + ((k1 - v) / 2) + a*dt / 2;

	//then k3

	//k4

	return k1;
}

float RKIntegration::integPositionFull(float s, float vn, float dt)
{
	return s + vn*(0.5*dt);
}
