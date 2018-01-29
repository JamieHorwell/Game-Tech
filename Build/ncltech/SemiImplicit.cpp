#include "SemiImplicit.h"



SemiImplicit::SemiImplicit()
{
}


SemiImplicit::~SemiImplicit()
{
}

Vector3 SemiImplicit::IntegVelocity(Vector3 v, Vector3 a, float dt)
{
	Vector3 vel = v + a*dt;
	vel = vel * 0.9999f;
	return vel;
}

Vector3 SemiImplicit::IntegPos(Vector3 pos, Vector3 vn1, float dt)
{
	return pos + vn1*dt;
}
