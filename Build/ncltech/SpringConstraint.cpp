#include "SpringConstraint.h"




SpringConstraint::SpringConstraint(PhysicsNode * obj1, PhysicsNode * obj2)
{
	pnodeA = obj1;
	pnodeB = obj2;

	restLength = (obj2->GetPosition() - obj1->GetPosition()).Length();
}

SpringConstraint::~SpringConstraint()
{
}

void SpringConstraint::ApplyImpulse()
{
	float currentDist = (pnodeB->GetPosition() - pnodeA->GetPosition()).Length();
	Vector3 springDir = pnodeB->GetPosition() - pnodeA->GetPosition();

	springDir.Normalise();

	float distDifference = currentDist - restLength;

	float ASpringVel = (pnodeA->GetLinearVelocity()*springDir).Length();
	float BSpringVel = (pnodeB->GetLinearVelocity()*-springDir).Length();



	float fMagA = (-(0.1 * distDifference) - 0.95*ASpringVel);
	float fMagB = (-(0.1 * distDifference) - 0.95*BSpringVel);

	Vector3 rForceA = springDir * fMagA;
	Vector3 rForceB = springDir * fMagB;

	pnodeA->addForce(rForceA);
	pnodeB->addForce(rForceB);
}
