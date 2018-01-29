#pragma once
#include "PhysicsNode.h"
#include "SphereCollisionShape.h"
#include "CuboidCollisionShape.h"
#include "../nclgl/Matrix4.h"

enum class boundingShapes { Cuboid, Sphere};

struct BParea {
	std::vector<PhysicsNode*> nodesInArea;
	Vector3 minVal;
	Vector3 maxVal;
	int depthLvl = 0;
	BParea(Vector3 min, Vector3 max, int depth = 0) : minVal(min), maxVal(max), depthLvl(depth) {}
	bool operator==(const BParea& rhs) const { return (minVal == rhs.minVal && maxVal == rhs.maxVal) ? true : false;  };
};


class BroadPhaseCulling
{
public:
	BroadPhaseCulling();
	~BroadPhaseCulling();

	void init(Vector3 minVal, Vector3 maxVal);

	

	static bool SphereSphereCollision(const PhysicsNode* a, const PhysicsNode* b);

	static  const boundingShapes boundingBox = boundingShapes::Sphere;

	void sortNode(PhysicsNode* p);

	void clear();

	bool splitArea(BParea a);

	static bool intersect(Vector3 minVal, Vector3 maxVal, Vector3 spherePos, float radius);

	std::vector<BParea> getBpAreas() { return bpAreas; };

	void DebugDraw();

protected:
	int maxNodes = 6;
	int maxDepth = 4;

	std::vector<BParea> bpAreas;

};

