#pragma once
#include "../nclgl/Vector3.h"
class RKIntegration
{
public:
	RKIntegration();
	~RKIntegration();

	static Vector3 integVelocity(Vector3 v, Vector3 a, float dt);




	static float integPositionFull(float s, float vn1, float dt);

};

