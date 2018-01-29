#pragma once
#include "PhysicsNode.h"
#include "DistanceConstraint.h"
#include "SphereCollisionShape.h"
#include "OpenCLprocessor.h"

class Cloth
{
public:
	Cloth(int width, int height, Vector3 offset);
	~Cloth();

	void constructClothHull();

	void DebugDraw();


	void Update(bool firstIteration, bool lastIteration);


private:
	std::vector<PhysicsNode*> pns;
	std::vector<DistanceConstraint*> cns;

	PhysicsNode** pnA;
	PhysicsNode** pnB;

	Vector4* Apos;
	Vector4* Bpos;

	Vector4* AVel;
	Vector4* BVel;

	float* massA;
	float* massB;

	float* targetLength;


	static Hull clothHull;

	int width;
	int height;

	OpenCLprocessor processor;
};

