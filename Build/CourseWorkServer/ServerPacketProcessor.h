#pragma once
#include <enet\enet.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\ServerMapRend.h>
#include <ncltech\ServerMazeGenerator.h>
#include <map>
#include <ncltech\ServerAStar.h>


class Raptor;

enum processingFlag {
	WAITING,CURRENTLYPROCESSING 
};



enum procType {
 GENMAP, STARTPOS, ENDPOS, SPAWNAVATAR
};


struct Avatar {
	Vector3 currentPos;
	Vector3 targetPos;
	Vector3 dirVec;
	std::list<const GraphNode*>::const_iterator it;
};


class ServerPacketProcessor
{
public:
	ServerPacketProcessor();
	~ServerPacketProcessor();

	//first packet should indicate what following packets represent
	bool processPacketInstruction(ENetPacket *pkt);


	//specific processing
	void processMapGenerate(ENetPacket *pkt);
	void processStartPos(ENetPacket *pkt);
	void processEndPos(ENetPacket *pkt);
	void sendMapDetails();
	void sendPathDetails();
	void updateAvatar(float dt);
	void updateRaptors(float dt);
	inline void spawnAvatar();

	void determineProcessType(ENetPacket *pkt);



	void setNetwork(NetworkBase& server) { this->server = &server; };

	bool getAvatarSpawned() { return avatarSpawned; };
	bool getMazeSpawned() { return mazeSpawned; };


protected:
	//packet processing
	processingFlag processState = processingFlag::WAITING;

	procType processType;

	std::map<std::string, procType> processMapper = { {"GENMAP",procType::GENMAP} };



	//server functionality
	NetworkBase* server;
	ServerMazeGenerator mapGen;
	ServerMapRend* mapRend;
	ServerAStar AstarSearch;
	GraphNode* start;
	GraphNode* end;


	Avatar* av;
	std::vector<Raptor*> raptors;

	bool avatarSpawned = false;
	bool mazeSpawned = false;
	bool finished = false;
};




