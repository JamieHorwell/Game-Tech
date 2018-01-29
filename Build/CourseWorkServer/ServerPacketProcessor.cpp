#include "ServerPacketProcessor.h"
#include "Raptor.h"


ServerPacketProcessor::ServerPacketProcessor()
{
}


ServerPacketProcessor::~ServerPacketProcessor()
{
}

bool ServerPacketProcessor::processPacketInstruction(ENetPacket *pkt)
{
	switch (processState) {
	case processingFlag::WAITING:
		determineProcessType(pkt);
		break;
	case processingFlag::CURRENTLYPROCESSING:
		if(processType == procType::GENMAP) processMapGenerate(pkt);
		if (processType == procType::STARTPOS) processStartPos(pkt);
		if (processType == procType::ENDPOS) processEndPos(pkt);
		if (processType == procType::SPAWNAVATAR) spawnAvatar();
		break;
	}
	return false;
}

void ServerPacketProcessor::processMapGenerate(ENetPacket *pkt)
{
	//expecting number to represent dimensions
	int mapDimensions;
	memcpy(&mapDimensions, pkt->data,sizeof(int));
	std::cout << "SERVER RECIEVED MAP DIMENSIONS: " << mapDimensions << "\n";
	mapGen.Generate(mapDimensions, 0.6);
	if (mapRend) {
		//delete mapRend;
		//mapRend = nullptr;
	}
	mapRend = new ServerMapRend(&mapGen);
	sendMapDetails();
	this->mazeSpawned = true;

}

void ServerPacketProcessor::processStartPos(ENetPacket * pkt)
{
	int pktData;
	memcpy(&pktData, pkt->data, sizeof(int));
	

	start = &mapGen.allNodes[pktData];
	if (end) {
		AstarSearch.FindBestPath(start, end);
		sendPathDetails();
	}
	processState = processingFlag::WAITING;
}

void ServerPacketProcessor::processEndPos(ENetPacket * pkt)
{
	int pktData;
	memcpy(&pktData, pkt->data, sizeof(int));


	end = &mapGen.allNodes[pktData];
	if (start) {
		AstarSearch.FindBestPath(start, end);
		sendPathDetails();
	}
	processState = processingFlag::WAITING;
}

void ServerPacketProcessor::sendMapDetails()
{
	//send flag that client should accept map
	int* flagData = new int(1);
	ENetPacket* packet = enet_packet_create(flagData, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);




	//send walls
	for (const WallDescriptor&w : mapRend->getWalls()) {
		packet = enet_packet_create(&w, sizeof(WallDescriptor), 0);
		enet_host_broadcast(server->m_pNetwork, 0, packet);
	}

	flagData = new int(0);
	packet = enet_packet_create(flagData, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);



	//send finish flag
	flagData = new int(2);
	packet = enet_packet_create(flagData, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);

	delete flagData;
	//finished sending map details across, now wait for next packet which could be anything
	processState = processingFlag::WAITING;
}

void ServerPacketProcessor::sendPathDetails()
{
	//send flag indicating route will be transmitted
	int flag = 3;
	ENetPacket* packet = enet_packet_create(&flag, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork,0,packet);

	//DO NOT FORGET THAT MAZE GRID IS SCALED UP BY FACTOR OF 5 CLIENT SIDE!!!!!
	Vector3 nodePos;
	for (const GraphNode * g : AstarSearch.GetFinalPath()) {
		nodePos.x = g->_pos.x;
		nodePos.y = 0;
		nodePos.z = g->_pos.y;

		packet = enet_packet_create(&nodePos,sizeof(Vector3), 0);
		enet_host_broadcast(server->m_pNetwork,0,packet);
	}

	int end = 0;
	packet = enet_packet_create(&end,sizeof(int),0);
	enet_host_broadcast(server->m_pNetwork,0,packet);

}

void ServerPacketProcessor::updateAvatar(float dt)
{
	float grid_scalar = 1.0f / (float)mapGen.GetSize();

	if (!finished) {
		//create avatar if not created
		if (!av) {
			av = new Avatar;
			av->it = AstarSearch.GetFinalPath().begin();
			av->currentPos = Vector3(((av->it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((av->it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
			++(av->it);
			av->targetPos = Vector3(((av->it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((av->it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
			av->dirVec = (av->targetPos - av->currentPos).Normalise();
			for (Raptor* r : raptors) {
				r->setAvatar(av);
			}
		}
		//check if avatar is at target position
		else if (av->currentPos == av->targetPos || (av->targetPos - av->currentPos).Length() < 0.08) {
			av->currentPos = av->targetPos;
			++(av->it);
			if (av->it == AstarSearch.GetFinalPath().end()) {
				finished = true;
			}
			else {
				av->targetPos = Vector3(((av->it)._Ptr->_Myval->_pos.x * 10 + 0.5f * 10) * grid_scalar, 0.1f, ((av->it)._Ptr->_Myval->_pos.y * 10 + 0.5f * 10) * grid_scalar);
				av->dirVec = (av->targetPos - av->currentPos).Normalise();
			}
			
		}
		//move avatar towards target position

		else {
			av->currentPos += av->dirVec * dt;
		}
	}
	
	int flag = 4;
	ENetPacket* packet = enet_packet_create(&flag,sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork,0,packet);

	//send packet indicating current avatar position
	 packet = enet_packet_create(&av->currentPos,sizeof(Vector3),0);
	 enet_host_broadcast(server->m_pNetwork,0,packet);
}

void ServerPacketProcessor::updateRaptors(float dt)
{
	//send raptor details
	int flag = 5;
	ENetPacket* packet = enet_packet_create(&flag, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);

	Vector3 pos;
	for (Raptor* r : raptors) {
		r->update(dt);

		
		pos = r->getCurrentPos();
		packet = enet_packet_create(&pos, sizeof(Vector3),0);
		enet_host_broadcast(server->m_pNetwork,0,packet);

	}
	//finished updates
	flag = 0;
	 packet = enet_packet_create(&flag, sizeof(int), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);
}

inline void ServerPacketProcessor::spawnAvatar()
{
		avatarSpawned = true; 	
		Raptor* r = new Raptor();
		r->setMazeGen(&mapGen);
		raptors.push_back(r);
}

void ServerPacketProcessor::determineProcessType(ENetPacket * pkt)
{
	int pktData;
	memcpy(&pktData, pkt->data, sizeof(int));
	printf("%i", pktData);

	if (pktData == procType::GENMAP) {
		processType = procType::GENMAP;
	}
	else if (pktData == procType::STARTPOS) {
		processType = procType::STARTPOS;
	}
	else if (pktData == procType::ENDPOS) {
		processType = procType::ENDPOS;
	}
	else if (pktData == procType::SPAWNAVATAR) {
		processType = procType::SPAWNAVATAR;
	}

	processState = processingFlag::CURRENTLYPROCESSING;

}



