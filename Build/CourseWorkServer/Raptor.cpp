#include "Raptor.h"



Raptor::Raptor()
{
	myState = std::bind(&Raptor::OnPatrolState, this, std::placeholders::_1);
	stateName = "Patrol";
}


Raptor::~Raptor()
{
}

void Raptor::update(float dt)
{


	if (myState) myState(dt);
	else NCLERROR("ERROR - NO STATE FOUND");
}

void Raptor::OnPatrolState(float dt)
{
	float grid_scalar = 1.0f / (float)mazeGen->GetSize();;


	if (currentPos == Vector3(0,0,0)) {
		int start = rand() % (mazeGen->getSize() * mazeGen->getSize());
		int end = rand() % (mazeGen->getSize() * mazeGen->getSize());
		search.FindBestPath(&(mazeGen->allNodes[start]), &(mazeGen->allNodes[end]));
		it = search.GetFinalPath().begin();
		currentPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
		++it;
		targetPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
		dirVec = (targetPos - currentPos).Normalise();
	}
	//check if weve reached target position
	if (currentPos == targetPos || (targetPos - currentPos).Length() < 0.05) {
		currentPos = targetPos;
		
		bool newPath = false;
		if (it == search.GetFinalPath().end()) {
			--it;
			const GraphNode* start = it._Ptr->_Myval;
			int end = rand() % (mazeGen->getSize() * mazeGen->getSize());
			search.FindBestPath(start, &(mazeGen->allNodes[end]));
			newPath = true;
			it = search.GetFinalPath().begin();
		}
		else {
		
			targetPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
			dirVec = (targetPos - currentPos).Normalise();
		}
		if (!newPath) {
			++it;
		}
		
	


	}
	currentPos += dirVec * dt;
	
	//now check whether to change states or not
	if (av) {
		if ((av->currentPos - currentPos).Length() < 5) {
			myState = std::bind(&Raptor::OnChaseState, this, std::placeholders::_1);
			//get graphnode of our current position
			it--;
			const GraphNode* current = it._Ptr->_Myval;
			//get graphnode of av current position
			--av->it;
			const GraphNode* end = av->it._Ptr->_Myval;
			++av->it;
			search.FindBestPath(current, end);
			it = search.GetFinalPath().begin();
			++it;
			targetPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
			dirVec = (targetPos - currentPos).Normalise();
		}
	}
}

void Raptor::OnChaseState(float dt)
{
	float grid_scalar = 1.0f / (float)mazeGen->GetSize();

	if (currentPos == targetPos || (targetPos - currentPos).Length() < 0.05) {
		++it;
		targetPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
		dirVec = (targetPos - currentPos).Normalise();
	}
	if (it == search.GetFinalPath().end()) {
		--it;
		const GraphNode* current = it._Ptr->_Myval;
		const GraphNode* end = av->it._Ptr->_Myval;
		search.FindBestPath(current, end);
		it = search.GetFinalPath().begin();
		++it;
		targetPos = Vector3(((it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
		dirVec = (targetPos - currentPos).Normalise();
	}


	currentPos += dirVec * dt;


	if ((av->currentPos - currentPos).Length() > 6) {
		myState = std::bind(&Raptor::OnPatrolState, this, std::placeholders::_1);
		--it;
		const GraphNode* start = it._Ptr->_Myval;
		int end = rand() % (mazeGen->getSize() * mazeGen->getSize());
		search.FindBestPath(start, &(mazeGen->allNodes[end]));
		it = search.GetFinalPath().begin();
	}


}

void Raptor::OnReturnState(float dt)
{
}

