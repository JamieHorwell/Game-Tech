#pragma once
#include "../ncltech/Scene.h"


#include <ncltech\NetworkBase.h>
#include "ClientPacketProcessor.h"



class MazeScene : public Scene
{
public:
	MazeScene(const std::string& friendly_name);
	
	~MazeScene();

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;
	void GenerateGround();

	void ProcessNetworkEvent(const ENetEvent& evnt);


	void debugDrawPath();

private:
	Mesh* wallmesh;



	NetworkBase network;
	ENetPeer* serverConnection;
	ClientPacketProcessor pktProcessor;

	float m_AccumTime;

	int mazeDimen = 10;
	int startPos = 0;
	int endPos = (10*10)-1;
};

