#include "PhysicsEngine.h"
#include "GameObject.h"
#include "CollisionDetectionSAT.h"
#include <nclgl\NCLDebug.h>
#include <nclgl\Window.h>
#include <omp.h>
#include <algorithm>
#include "Cloth.h"

void PhysicsEngine::SetDefaults()
{
	//Variables set here /will/ be reset with each scene
	updateTimestep = 1.0f / 60.f;
	updateRealTimeAccum = 0.0f;
	gravity = Vector3(0.0f, -9.1f, 0.0f);
	dampingFactor = 0.998f;
}

PhysicsEngine::PhysicsEngine()
{
	//Variables set here will /not/ be reset with each scene
	isPaused = false;
	debugDrawFlags = DEBUGDRAW_FLAGS_MANIFOLD | DEBUGDRAW_FLAGS_CONSTRAINT;

	SetDefaults();
}

PhysicsEngine::~PhysicsEngine()
{
	RemoveAllPhysicsObjects();
}

void PhysicsEngine::AddPhysicsObject(PhysicsNode* obj)
{
	physicsNodes.push_back(obj);
	BpOct.sortNode(obj);
}

void PhysicsEngine::RemovePhysicsObject(PhysicsNode* obj)
{
	//Lookup the object in question
	auto found_loc = std::find(physicsNodes.begin(), physicsNodes.end(), obj);

	//If found, remove it from the list
	if (found_loc != physicsNodes.end())
	{
		physicsNodes.erase(found_loc);
	}
}

void PhysicsEngine::RemoveAllPhysicsObjects()
{
	//Delete and remove all constraints/collision manifolds
	for (Constraint* c : constraints)
	{
		delete c;
	}
	constraints.clear();

	for (Manifold* m : manifolds)
	{
		delete m;
	}
	manifolds.clear();


	//Delete and remove all physics objects
	// - we also need to inform the (possibly) associated game-object
	//   that the physics object no longer exists
	for (PhysicsNode* obj : physicsNodes)
	{
		if (obj->GetParent()) obj->GetParent()->SetPhysics(NULL);
		delete obj;
	}
	physicsNodes.clear();
	BpOct.clear();
}


void PhysicsEngine::Update(float deltaTime)
{
	//The physics engine should run independantly to the renderer
	// - As our codebase is currently single threaded we just need
	//   a way of calling "UpdatePhysics()" at regular intervals
	//   or multiple times a frame if the physics timestep is higher
	//   than the renderers.
	const int max_updates_per_frame = 5;

	if (!isPaused)
	{
		updateRealTimeAccum += deltaTime;
		for (int i = 0; (updateRealTimeAccum >= updateTimestep) && i < max_updates_per_frame; ++i)
		{
			updateRealTimeAccum -= updateTimestep;

			//Additional IsPaused check here incase physics was paused inside one of it's components for debugging or otherwise
			if (!isPaused) UpdatePhysics();
		}

		if (updateRealTimeAccum >= updateTimestep)
		{
			NCLDebug::Log("Physics too slow to run in real time!");
			//Drop Time in the hope that it can continue to run faster the next frame
			updateRealTimeAccum = 0.0f;
		}
	}
}


void PhysicsEngine::UpdatePhysics()
{
	for (Manifold* m : manifolds)
	{
		delete m;
	}
	manifolds.clear();

	perfUpdate.UpdateRealElapsedTime(updateTimestep);
	perfBroadphase.UpdateRealElapsedTime(updateTimestep);
	perfNarrowphase.UpdateRealElapsedTime(updateTimestep);
	perfSolver.UpdateRealElapsedTime(updateTimestep);




	//A whole physics engine in 6 simple steps =D

	//-- Using positions from last frame --
	//1. Broadphase Collision Detection (Fast and dirty)
	perfBroadphase.BeginTimingSection();
	BroadPhaseCollisions();
	perfBroadphase.EndTimingSection();

	//2. Narrowphase Collision Detection (Accurate but slow)
	perfNarrowphase.BeginTimingSection();
	NarrowPhaseCollisions();
	perfNarrowphase.EndTimingSection();


	//3. Initialize Constraint Params (precompute elasticity/baumgarte factor etc)
	//Optional step to allow constraints to 
	// precompute values based off current velocities 
	// before they are updated loop below.
	for (Manifold* m : manifolds) m->PreSolverStep(updateTimestep);
	for (Constraint* c : constraints) c->PreSolverStep(updateTimestep);

	


	//4. Update Velocities
	perfUpdate.BeginTimingSection();
	for (PhysicsNode* obj : physicsNodes) {
		obj->IntegrateForVelocity(updateTimestep);
	}
	perfUpdate.EndTimingSection();

	//5. Constraint Solver
	perfSolver.BeginTimingSection();
	for (Constraint* c : constraints) c->ApplyImpulse();
	perfSolver.EndTimingSection();
	/*if (cloth) {
		cloth->Update(true, false);
	}*/


	for (size_t i = 0; i < SOLVER_ITERATIONS; ++i) {
		for (Manifold* m : manifolds) m->ApplyImpulse();
		for (Constraint* c : constraints) c->ApplyImpulse();
		
		/*if (cloth) {
			if (i == SOLVER_ITERATIONS - 1) {
				cloth->Update(false, true);
			}
			else {
				cloth->Update(false, false);
			}
			
		}*/
	}
	
	//6. Update Positions (with final 'real' velocities)
	perfUpdate.BeginTimingSection();
	for (PhysicsNode* obj : physicsNodes) obj->IntegrateForPosition(updateTimestep);
	perfUpdate.EndTimingSection();
}

void PhysicsEngine::BroadPhaseCollisions()
{
	broadphaseColPairs.clear();

	PhysicsNode *pnodeA, *pnodeB;
	//	The broadphase needs to build a list of all potentially colliding objects in the world,
	//	which then get accurately assesed in narrowphase. If this is too coarse then the system slows down with
	//	the complexity of narrowphase collision checking, if this is too fine then collisions may be missed.
	for (PhysicsNode * p : physicsNodes) {
		if (p->GetLinearVelocity().Length() > 0) {
			BpOct.sortNode(p);
		}
	}

	//	Brute force approach.
	//  - For every object A, assume it could collide with every other object.. 
	//    even if they are on the opposite sides of the world.
	if (physicsNodes.size() > 0)
	{
		for (BParea &p : BpOct.getBpAreas()) {
			
			for (size_t i = 0; i < p.nodesInArea.size() - 1 && p.nodesInArea.size() > 0; ++i)
			{
				for (size_t j = i + 1; j < p.nodesInArea.size(); ++j)
				{
					pnodeA = p.nodesInArea[i];
					pnodeB = p.nodesInArea[j];

					//lets check collisions with broadphase shapes



					//Check they both atleast have collision shapes
					if (BroadPhaseCulling::SphereSphereCollision(pnodeA, pnodeB))
					{
						CollisionPair cp;
						cp.pObjectA = pnodeA;
						cp.pObjectB = pnodeB;
						//broadphaseColPairs.push_back(cp);
						broadphaseColPairs.emplace(cp);
					}

				}
			}


		}
		
	}
}


void PhysicsEngine::NarrowPhaseCollisions()
{
	//reset collision data and color
	for (PhysicsNode* p : physicsNodes) {
		p->setCollided(false);
		//p->GetParent()->renderNode->SetColorRecursive(Vector4(1, 0, 1, 1));
	}

	if (broadphaseColPairs.size() > 0)
	{
		//Collision data to pass between detection and manifold generation stages.
		CollisionData colData;

		//Collision Detection Algorithm to use
		CollisionDetectionSAT colDetect;

		// Iterate over all possible collision pairs and perform accurate collision detection
		for (CollisionPair  cp : broadphaseColPairs)
		{
			//CollisionPair& cp = broadphaseColPairs.;

			CollisionShape *shapeA = cp.pObjectA->GetCollisionShape();
			CollisionShape *shapeB = cp.pObjectB->GetCollisionShape();

			//iterate over all collShapes of each object
			for (int i = 0; i < cp.pObjectA->getCollNodes().size(); ++i) {
				for (int j = 0; j < cp.pObjectB->getCollNodes().size(); ++j) {
					shapeA = cp.pObjectA->getCollNodes()[i];
					shapeB = cp.pObjectB->getCollNodes()[j];


					//TODO, COLDETECT TAKE IN JUST SHAPES, USE COMBINED TRANSFORMATIONS FOR RETRIEVING COL DATA
					colDetect.BeginNewPair(cp.pObjectA, cp.pObjectB, shapeA, shapeB);


					if (colDetect.AreColliding(&colData))
					{
						//Note: As at the end of tutorial 4 we have very little to do, this is a bit messier
						//      than it should be. We now fire oncollision events for the two objects so they
						//      can handle AI and also optionally draw the collision normals to see roughly
						//      where and how the objects are colliding.

						//Draw collision data to the window if requested
						// - Have to do this here as colData is only temporary. 
						if (debugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS)
						{
							NCLDebug::DrawPointNDT(colData._pointOnPlane, 0.1f, Vector4(0.5f, 0.5f, 1.0f, 1.0f));
							NCLDebug::DrawThickLineNDT(colData._pointOnPlane, colData._pointOnPlane - colData._normal * colData._penetration, 0.05f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
						}

						cp.pObjectA->setCollided(true);
						cp.pObjectB->setCollided(true);
						//colour the colliding shapes

						//cp.pObjectA->GetParent()->renderNode->SetColorRecursive(Vector4(1, 1, 1, 1));
						//cp.pObjectB->GetParent()->renderNode->SetColorRecursive(Vector4(1, 1, 1, 1));



						//Check to see if any of the objects have a OnCollision callback that dont want the objects to physically collide
						bool okA = cp.pObjectA->FireOnCollisionEvent(cp.pObjectA, cp.pObjectB);
						bool okB = cp.pObjectB->FireOnCollisionEvent(cp.pObjectB, cp.pObjectA);



						if (okA && okB)
						{
							/* TUTORIAL 5 CODE */
							Manifold* manifold = new Manifold();
							manifold->Initiate(cp.pObjectA, cp.pObjectB);

							colDetect.GenContactPoints(manifold);

							if (manifold->contactPoints.size() > 0) {
								manifolds.push_back(manifold);
							}
							else {
								delete manifold;
							}
						}
					}


				}
			}
			


			//colDetect.BeginNewPair(
			//	cp.pObjectA,
			//	cp.pObjectB,
			//	cp.pObjectA->GetCollisionShape(),
			//	cp.pObjectB->GetCollisionShape());

			////--TUTORIAL 4 CODE--
			//// Detects if the objects are colliding
			//if (colDetect.AreColliding(&colData))
			//{
			//	//Note: As at the end of tutorial 4 we have very little to do, this is a bit messier
			//	//      than it should be. We now fire oncollision events for the two objects so they
			//	//      can handle AI and also optionally draw the collision normals to see roughly
			//	//      where and how the objects are colliding.

			//	//Draw collision data to the window if requested
			//	// - Have to do this here as colData is only temporary. 
			//	if (debugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS)
			//	{
			//		NCLDebug::DrawPointNDT(colData._pointOnPlane, 0.1f, Vector4(0.5f, 0.5f, 1.0f, 1.0f));
			//		NCLDebug::DrawThickLineNDT(colData._pointOnPlane, colData._pointOnPlane - colData._normal * colData._penetration, 0.05f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
			//	}

			//	cp.pObjectA->setCollided(true);
			//	cp.pObjectB->setCollided(true);
			//	//colour the colliding shapes

			//	//cp.pObjectA->GetParent()->renderNode->SetColorRecursive(Vector4(1, 1, 1, 1));
			//	//cp.pObjectB->GetParent()->renderNode->SetColorRecursive(Vector4(1, 1, 1, 1));



			//	//Check to see if any of the objects have a OnCollision callback that dont want the objects to physically collide
			//	bool okA = cp.pObjectA->FireOnCollisionEvent(cp.pObjectA, cp.pObjectB);
			//	bool okB = cp.pObjectB->FireOnCollisionEvent(cp.pObjectB, cp.pObjectA);



			//	if (okA && okB)
			//	{
			//		/* TUTORIAL 5 CODE */
			//		Manifold* manifold = new Manifold();
			//		manifold->Initiate(cp.pObjectA, cp.pObjectB);

			//		colDetect.GenContactPoints(manifold);

			//		if (manifold->contactPoints.size() > 0) {
			//			manifolds.push_back(manifold);
			//		}
			//		else {
			//			delete manifold;
			//		}
			//	}
			//}



		}

	}
}


void PhysicsEngine::DebugRender()
{
	// Draw all collision manifolds
	if (debugDrawFlags & DEBUGDRAW_FLAGS_MANIFOLD)
	{
		for (Manifold* m : manifolds)
		{
			m->DebugDraw();
		}
	}

	// Draw all constraints
	if (debugDrawFlags & DEBUGDRAW_FLAGS_CONSTRAINT)
	{
		for (Constraint* c : constraints)
		{
			c->DebugDraw();
		}
	}

	// Draw all associated collision shapes
	if (debugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES)
	{
		for (PhysicsNode* obj : physicsNodes)
		{
			/*if (obj->GetCollisionShape() != NULL)
			{
				obj->GetCollisionShape()->DebugDraw();
			}*/

			if (obj->getCollNodes().size() > 0) {
				for (CollisionShape* colNode : obj->getCollNodes()) {
					colNode->DebugDraw();
				}
			}

		}
	}

	if (debugDrawFlags & DEBUGDRAW_FLAGS_BROADPHASESHAPES) {
		for (PhysicsNode* obj : physicsNodes) {
			if (obj->GetBroadPhaseShape() != NULL) {
				obj->GetBroadPhaseShape()->DebugDraw();
			}
		}
	}

	if (debugDrawFlags & DEBUGDRAW_FLAGS_BROADPHASEOCT) {
		BpOct.DebugDraw();
	}
}