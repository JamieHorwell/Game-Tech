#pragma once
#include <functional>
#include <nclgl\Vector3.h>
#include <ncltech\ServerAStar.h>
#include "ServerPacketProcessor.h"

typedef std::function<void(float dt)> OnStateUpdateCallback;

class Raptor
{
public:
	Raptor();
	~Raptor();

	void update(float dt);


	void OnPatrolState(float dt);
	void OnChaseState(float dt);
	void OnReturnState(float dt);


	Vector3 getCurrentPos() { return currentPos; };


	void setMazeGen(ServerMazeGenerator* mazeGen) { this->mazeGen = mazeGen; };
	void setAvatar(Avatar* av) { this->av = av; };

protected:
	OnStateUpdateCallback myState;
	std::string stateName;

	Vector3 currentPos;
	Vector3 targetPos;
	Vector3 dirVec;
	ServerAStar search;
	std::list<const GraphNode*>::const_iterator it;


	Avatar* av;
	ServerMazeGenerator* mazeGen;
};

