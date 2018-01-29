/******************************************************************************
Class: DistanceConstraint
Implements:
Author:
Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

Manages a distance constraint between two objects, ensuring the two objects never
seperate. It works on the velocity level, enforcing the constraint:
dot([(velocity of B) - (velocity of A)], normal) = zero

Which is the same as saying, if the velocity of the two objects in the direction of the constraint is zero
then we can assert that the two objects will not move further or closer together and thus satisfy the constraint.

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Constraint.h"
#include "PhysicsEngine.h"
#include <nclgl\NCLDebug.h>

class DistanceConstraint : public Constraint
{
public:
	DistanceConstraint(PhysicsNode* obj1, PhysicsNode* obj2,
		const Vector3& globalOnA, const Vector3& globalOnB)
	{
		pnodeA = obj1;
		pnodeB = obj2;

		//Set the preferred distance of the constraint to enforce 
		// (ONLY USED FOR BAUMGARTE)
		// - Ideally we only ever work at the velocity level, so satifying (velA-VelB = 0)
		//   is enough to ensure the distance never changes.
		Vector3 ab = globalOnB - globalOnA;
		targetLength = ab.Length();

		//Get the local points (un-rotated) on the two objects where the constraint should
		// be attached relative to the objects center. So when we rotate the objects
		// the constraint attachment point will rotate with it.
		Vector3 r1 = (globalOnA - pnodeA->GetPosition());
		Vector3 r2 = (globalOnB - pnodeB->GetPosition());
		relPosA = Matrix3::Transpose(pnodeA->GetOrientation().ToMatrix3()) * r1;
		relPosB = Matrix3::Transpose(pnodeB->GetOrientation().ToMatrix3()) * r2;
	}

	int indexA;
	int indexB;


	//Solves the constraint and applies a velocity impulse to the two
	// objects in order to satisfy the constraint.
	virtual void ApplyImpulse() override
	{
		////get constraing vars based on object A/B's position/rotation
		////Vector3 r1 = pnodeA->GetOrientation().ToMatrix3() * relPosA * Vector3(0,0,0);
		////Vector3 r2 = pnodeB->GetOrientation().ToMatrix3() * relPosB * Vector3(0, 0, 0);

		//Vector3 globalOnA = pnodeA->GetPosition();
		//Vector3 globalOnB = pnodeB->GetPosition();

		////get vector between two contact points

		//Vector3 ab = globalOnB - globalOnA;
		//Vector3 abn = ab;
		//abn.Normalise();

		////compute velocity of objects A and B at point of contact

		//Vector3 v0 = pnodeA->GetLinearVelocity(); //+ Vector3::Cross(pnodeA->GetAngularVelocity(), r1);
		//Vector3 v1 = pnodeB->GetLinearVelocity(); //+ Vector3::Cross(pnodeB->GetAngularVelocity(), r2);

		////relative velocity in constraint direction
		//float abnVel = Vector3::Dot(v0 - v1, abn);

		////compute mass of constraint
		////e.g. how difficult it is to move two objects in direction of the constraint
		//float invConstraintMassLin = pnodeA->GetInverseMass() + pnodeB->GetInverseMass();

		////float invConstrainMassRot = Vector3::Dot(abn,
		////	Vector3::Cross(pnodeA->GetInverseInertia()
		////		* Vector3::Cross(r1, abn), r1)
		////	+ Vector3::Cross(pnodeB->GetInverseInertia() *
		////		Vector3::Cross(r2, abn), r2));

		//float invConstrainMassRot = 0;

		//float constraintMass = invConstraintMassLin + invConstrainMassRot;

		//if (constraintMass > 0.0f) {
		//	//add energy to system to counter slight solving errors

		//	float b = 0.0f;

		//	float distance_offset = ab.Length() - targetLength;
		//	float baumgarte_scalar = 0.1f;
		//	b = -(baumgarte_scalar / PhysicsEngine::Instance()->GetDeltaTime()) * distance_offset;

		//	//compute velocity impulse

		//	float jn = (-(abnVel + b) / constraintMass);

		//	pnodeA->SetLinearVelocity(pnodeA->GetLinearVelocity() + abn * (pnodeA->GetInverseMass() * jn));
		//	pnodeB->SetLinearVelocity(pnodeB->GetLinearVelocity() - abn * (pnodeB->GetInverseMass() * jn));


		//	/*pnodeA->SetAngularVelocity(pnodeA->GetAngularVelocity() + pnodeA->GetInverseInertia() * Vector3::Cross(r1, abn * jn));
		//	pnodeB->SetAngularVelocity(pnodeB->GetAngularVelocity() - pnodeB->GetInverseInertia() * Vector3::Cross(r2, abn * jn));*/

		//}

		Vector3 globalOnA = pnodeA->GetPosition();
		Vector3 globalOnB = pnodeB->GetPosition();

		Vector3 ab = globalOnB - globalOnA;


		Vector3 abn = ab;
		abn.Normalise();


		Vector3 v0v1 = pnodeA->GetLinearVelocity() - pnodeB->GetLinearVelocity();
		float abnVel = Vector3::Dot(v0v1, abn);

		float invConstraintMassLin = pnodeA->GetInverseMass() + pnodeB->GetInverseMass();


		if (invConstraintMassLin > 0.0f) {
			float b = 0.0f;
			//TODO: WRITE LENGTH FUNCTION
			float distOffset = ab.Length() - targetLength;
			float baumgarte_scalar = 0.1f;
			b = -(baumgarte_scalar / 0.0013) * distOffset;

			float jn = (-(abnVel + b) / invConstraintMassLin);

			//set lin vel of A
			pnodeA->SetLinearVelocity(pnodeA->GetLinearVelocity() + (abn * (pnodeA->GetInverseMass() * jn)));
			//set lin vel of B
			pnodeB->SetLinearVelocity(pnodeB->GetLinearVelocity() - (abn * (pnodeB->GetInverseMass() * jn)));
		}

	}

	//Draw the constraint visually to the screen for debugging
	virtual void DebugDraw() const
	{
		Vector3 globalOnA = pnodeA->GetOrientation().ToMatrix3() * relPosA + pnodeA->GetPosition();
		Vector3 globalOnB = pnodeB->GetOrientation().ToMatrix3() * relPosB + pnodeB->GetPosition();

		NCLDebug::DrawThickLine(globalOnA, globalOnB, 0.02f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnA, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnB, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
	}

	PhysicsNode *pnodeA, *pnodeB;

	float   targetLength;
protected:
	

	Vector3 relPosA;
	Vector3 relPosB;


};