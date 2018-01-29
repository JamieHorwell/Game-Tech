#include "BroadPhaseCulling.h"



BroadPhaseCulling::BroadPhaseCulling()
{
	maxNodes = 3;
}


BroadPhaseCulling::~BroadPhaseCulling()
{
}

void BroadPhaseCulling::init(Vector3 minVal, Vector3 maxVal)
{
	Vector3 areaSize = (maxVal - minVal)/2;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			for (int z = 0; z < 2; ++z) {
				Vector3 minVal(minVal.x+(areaSize.x * x),minVal.y+(areaSize.y*y),minVal.z+(areaSize.z*z));
				Vector3 maxVal(maxVal.x-(areaSize.x*(1-x)),maxVal.y-(areaSize.y*(1-y)),maxVal.z-(areaSize.z*(1-z)));
				bpAreas.push_back(BParea(minVal,maxVal));
			}
		}
	}
}

bool BroadPhaseCulling::SphereSphereCollision(const PhysicsNode* a, const PhysicsNode* b)
{
	if (boundingBox == boundingShapes::Sphere) {
		SphereCollisionShape* aSphr = dynamic_cast<SphereCollisionShape*>(a->GetBroadPhaseShape());
		SphereCollisionShape* bSphr = dynamic_cast<SphereCollisionShape*>(b->GetBroadPhaseShape());

		if (aSphr && bSphr) {
			if ((b->GetPosition() - a->GetPosition()).Length() <= aSphr->GetRadius() + bSphr->GetRadius()) return true;
		}
	}
	return false;
}

void BroadPhaseCulling::sortNode(PhysicsNode * p)
{
	//collision detection with axis aligned boundingBox of Octrees
	if (p->GetBroadPhaseShape()) {
		
		float radius = dynamic_cast<SphereCollisionShape*>(p->GetBroadPhaseShape())->GetRadius();
		Vector3 centre = p->GetPosition();
		//use iterator instead 
		for (int i = 0; i < bpAreas.size(); ++i) {
			if (intersect(bpAreas[i].minVal, bpAreas[i].maxVal, centre, radius)) {
				std::vector<PhysicsNode*>::iterator index = std::find(bpAreas[i].nodesInArea.begin(), bpAreas[i].nodesInArea.end(), p);
				if (index != bpAreas[i].nodesInArea.end()) bpAreas[i].nodesInArea.erase(index);
				bpAreas[i].nodesInArea.push_back(p);
				splitArea((bpAreas.at(i)));
			}
			else {
				//check if area had it previously and remove it?
			std::vector<PhysicsNode*>::iterator index = std::find(bpAreas[i].nodesInArea.begin(), bpAreas[i].nodesInArea.end(), p);
			if (index != bpAreas[i].nodesInArea.end()) bpAreas[i].nodesInArea.erase(index);
			}
		}
	}
	
}

void BroadPhaseCulling::clear()
{
	bpAreas.erase(bpAreas.begin(), bpAreas.end());
	bpAreas.clear();
}

bool BroadPhaseCulling::splitArea(BParea a)
{
	if (a.nodesInArea.size() >= maxNodes && a.depthLvl < maxDepth) {
		Vector3 areaSize = (a.maxVal - a.minVal) / 2;
		for (int x = 0; x < 2; ++x) {
			for (int y = 0; y < 2; ++y) {
				for (int z = 0; z < 2; ++z) {
					Vector3 minVal(a.minVal.x + (areaSize.x * x), a.minVal.y + (areaSize.y*y), a.minVal.z + (areaSize.z*z));
					Vector3 maxVal(a.maxVal.x - (areaSize.x*(1 - x)), a.maxVal.y - (areaSize.y*(1 - y)), a.maxVal.z - (areaSize.z*(1 - z)));
					bpAreas.push_back(BParea(minVal, maxVal, a.depthLvl + 1));
					
				}
			}
		}
		std::vector<BParea>::iterator index = std::find(bpAreas.begin(), bpAreas.end(), a);
		//sort nodes again that was in this partition
		
		BParea a = *(index);
		bpAreas.erase(index);
		for (PhysicsNode* p : a.nodesInArea) { sortNode(p); };
		return true;
	}
	else return false;
}



bool BroadPhaseCulling::intersect(Vector3 minVal, Vector3 maxVal, Vector3 spherePos, float radius)
{
	if (spherePos.x - radius < maxVal.x && spherePos.x + radius > minVal.x) {
		if (spherePos.y - radius < maxVal.y && spherePos.y + radius > minVal.y) {
			if (spherePos.z - radius < maxVal.z && spherePos.z + radius > minVal.z) {
				return true;
			}
		}
	}
	return false;
}

void BroadPhaseCulling::DebugDraw()
{
	
	for (BParea cube : bpAreas) {
		
		Vector3 scale = (cube.maxVal - cube.minVal)/2;

		Matrix4 transform = Matrix4::Translation(cube.minVal + ((cube.maxVal - cube.minVal)/2)) * Matrix4::Scale(Vector3(scale.x,scale.y,scale.z));
		int numNodes = cube.nodesInArea.size();
		float full = numNodes / (float) maxNodes;
		CuboidCollisionShape::getcubeHull().DebugDraw(transform,Vector4(1-full,1-full,1,0));
	}
}
