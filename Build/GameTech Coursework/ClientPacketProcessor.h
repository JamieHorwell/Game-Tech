#pragma once
#include <enet\enet.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\Scene.h>

#include <ncltech\ServerMapRend.h>
#include <nclgl\OBJMesh.h>

enum processType {
	WAITING, MAPGEN, MAPFINISH, PATHDETAILS, AVATARUPDATE, RAPTORUPDATE
};


class ClientPacketProcessor
{
public:
	ClientPacketProcessor(Scene* scene);
	~ClientPacketProcessor();


	void setServerConn(ENetPeer* serverCon) { this->serverConnection = serverCon; };

	//RECIEVING
	void recieveMapGen(ENetPacket *pkt);
	void buildPath(ENetPacket *pkt);
	void processPacket(ENetPacket *pkt);
	void determinePacketInst(ENetPacket *pkt);
	void updateAvatar(ENetPacket *pkt);
	void updateRaptors(ENetPacket *pkt);


	//SENDING
	void sendStartPos(int start);
	void sendGoalPos(int end);
	void sendMapGen(int dimen);
	void spawnAvatar(int startPos);

	//FUNCTIONALITY
	void setWallMesh(Mesh* mesh) { this->wallmesh = mesh; };
	void setMazeSize(int size) { mazeSize = size; };
	void buildMapWalls();
	void deleteMaze() { mazeNode = nullptr; };


	std::vector<Vector3> getPath() { return path; };

	int getMazeSize() { return mazeSize; };


protected:
	Scene* scene;
	//maze info
	Mesh* wallmesh;
	RenderNode* mazeNode;
	//for debugging
	std::vector<Vector3> path;

	std::vector<WallDescriptor> wds;

	int mazeSize = 10;


	processType currentProc = WAITING;


	ENetPeer* serverConnection;

	int raptorCount = 0;
};

