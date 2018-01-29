#pragma once
#include "Constraint.h"
class SpringConstraint :
	public Constraint
{
public:
	SpringConstraint(PhysicsNode* obj1, PhysicsNode* obj2);
	~SpringConstraint();



	virtual void ApplyImpulse() override;


protected:
	PhysicsNode* pnodeA;
	PhysicsNode* pnodeB;


	float restLength;

};

