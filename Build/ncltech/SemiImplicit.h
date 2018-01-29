#pragma once
#include "../nclgl/Vector3.h"
class SemiImplicit
{
public:
	SemiImplicit();
	~SemiImplicit();

	static Vector3 IntegVelocity(Vector3 v, Vector3 a, float dt);

	static Vector3 IntegPos(Vector3 pos, Vector3 vn1, float dt);
};

