#include "Cloth.h"
#include "PhysicsEngine.h"



Hull Cloth::clothHull = Hull();

Cloth::Cloth(int width, int height, Vector3 offset)
{
	//nodes
	this->width = width;
	this->height = height;


	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			Vector3 pos = offset + Vector3(x*0.5, y*0.5, 0);
			PhysicsNode* pn = new PhysicsNode();
			if (y == height-1) {
				pn->SetInverseMass(0);
			}
			else {
				pn->SetInverseMass(10);
			}
			pn->SetPosition(pos);
			pn->AddColShape(new SphereCollisionShape(0.2));
			pn->SetBroadphaseShape(new SphereCollisionShape(0.2));
			pns.push_back(pn);
			PhysicsEngine::Instance()->AddPhysicsObject(pn);
		}
	}


	//now create distant constraints of each node
	//first create distance constraints connecting row
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height - 1; ++y) {
			int index = y + (x*width);
			DistanceConstraint* dn = new DistanceConstraint(pns[index],pns[index+1],pns[index]->GetPosition(),pns[index+1]->GetPosition());
			dn->indexA = index;
			dn->indexB = index + 1;
			cns.push_back(dn);
			PhysicsEngine::Instance()->AddConstraint(dn);
		}
	}

	for (int x = 0; x < width - 1; ++x) {
		for (int y = 0; y < height; ++y) {
			int index = y + (x*width);
			DistanceConstraint* dn = new DistanceConstraint(pns[index], pns[index + width], pns[index]->GetPosition(), pns[index + width]->GetPosition());
			dn->indexA = index;
			dn->indexB = index + width;
			cns.push_back(dn);
			PhysicsEngine::Instance()->AddConstraint(dn);
		}
	}

	//diagonol down left
	for (int x = 0; x < width - 1; ++x) {
		for (int y = 0; y < height - 1; ++y) {
			int index = y + (x*width);
			int indexDR = index + width + 1;
			DistanceConstraint* dn = new DistanceConstraint(pns[index], pns[indexDR], pns[index]->GetPosition(), pns[indexDR]->GetPosition());
			dn->indexA = index;
			dn->indexB = indexDR;
			cns.push_back(dn);
			PhysicsEngine::Instance()->AddConstraint(dn);
		}
	}

	for (int x = 0; x < width - 1; ++x) {
		for (int y = 1; y < height; ++y) {
			int index = y + (x*width);
			int indexDL = index + width - 1;
			DistanceConstraint* dn = new DistanceConstraint(pns[index], pns[indexDL], pns[index]->GetPosition(), pns[indexDL]->GetPosition());
			dn->indexA = index;
			dn->indexB = indexDL;
			cns.push_back(dn);
			PhysicsEngine::Instance()->AddConstraint(dn);
		}
	}

	
	constructClothHull();

	//get arrays with various data
	//HAVE TO INPUT VECTOR3 as VECTOR4 TO OPENCL BECAUSE OF FLOAT3 and FLOAT4 BEING THE BLOODY SAME IN OPENCL
	pnA = new PhysicsNode*[cns.size()];
	pnB = new PhysicsNode*[cns.size()];
	 Apos = new Vector4[cns.size()];
	 Bpos = new Vector4[cns.size()];
	 AVel = new Vector4[cns.size()];
	 BVel = new Vector4[cns.size()];
	 massA = new float[cns.size()];
	 massB = new float[cns.size()];
	 targetLength = new float[cns.size()];

	int count = 0;
	for (DistanceConstraint * dc : cns) {
		pnA[count] = dc->pnodeA;
		pnB[count] = dc->pnodeB;
		Apos[count] = dc->pnodeA->GetPosition();
		Bpos[count] = dc->pnodeB->GetPosition();
		AVel[count] = dc->pnodeA->GetLinearVelocity();
		BVel[count] = dc->pnodeB->GetLinearVelocity();
		massA[count] = dc->pnodeA->GetInverseMass();
		massB[count] = dc->pnodeB->GetInverseMass();
		targetLength[count] = dc->targetLength;
			++count;
	}


	processor.initBuffers(cns.size(),Apos,Bpos,AVel,BVel,massA,massB,targetLength);
}

Cloth::~Cloth()
{
}

void Cloth::constructClothHull()
{
	std::vector<int*>faces;

	clothHull = Hull();

	//construct square face for every 
	for (int x = 0; x < width - 1; ++x) {
		for (int y = 0; y < height - 1; ++y) {
			int index = y + (x*width);
			int indexR = index + 1;
			int indexD = index + width;
			int indexDR = indexR + width;
			faces.push_back(new int[4]{ indexD,index,indexR,indexDR });
		}

	}


	for (PhysicsNode* pn : pns) {
		clothHull.AddVertex(pn->GetPosition());
	}



	for (int* face : faces) {
		clothHull.AddFace(Vector3(0,0,0),4,face);
	}


	for (int i = 0; i < faces.size(); ++i) {
		delete faces[i];
	}
	faces.clear();

}

void Cloth::DebugDraw() 
{
	for (int i = 0; i < pns.size(); ++i) {
		clothHull.setVertexPos(i, pns[i]->GetPosition());
	}


	//update all ther vertexes
	Matrix4 transform = Matrix4::Translation(Vector3(0, 0, 0));
	clothHull.DebugDraw(transform);
}

void Cloth::Update(bool firstIteration, bool lastIteration)
{
	


	if (firstIteration) {
		for (int i = 0; i < cns.size(); ++i) {
			int aIndex = (int)(cns[i]->pnodeA);
			intptr_t as = (intptr_t)(cns[i]->pnodeA);
			AVel[i] = cns[i]->pnodeA->GetLinearVelocity();
			BVel[i] = cns[i]->pnodeB->GetLinearVelocity();
			
			Apos[i] = cns[i]->pnodeA->GetPosition();
			Bpos[i] = cns[i]->pnodeB->GetPosition();
		}
	}
	else {
		for (int i = 0; i < cns.size(); ++i) {
			AVel[i] = cns[i]->pnodeA->GetLinearVelocity();
			BVel[i] = cns[i]->pnodeB->GetLinearVelocity();
		}
	}

	//feed updated values into buffers
	if (firstIteration) {
		processor.updateBuffers(cns.size(),Apos,Bpos, AVel, BVel);
	}

	//processor.updateConstraints(cns.size(), lastIteration);
	processor.updateConstraints(cns.size(), AVel, BVel);

	//update pointers with new data
	//if (lastIteration) {
		processor.readBack(cns.size(),AVel,BVel);
		for (int i = 0; i < cns.size(); ++i) {
			pnA[i]->SetLinearVelocity(pnA[i]->GetLinearVelocity() + AVel[i].ToVector3());
			pnB[i]->SetLinearVelocity(pnB[i]->GetLinearVelocity() - BVel[i].ToVector3());
		}
	//}

}
