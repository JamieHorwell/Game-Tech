#include "ClientPacketProcessor.h"
#include <string>
#include <ncltech\CommonUtils.h>



ClientPacketProcessor::ClientPacketProcessor(Scene * scene)
{
	this->scene = scene;
	
}

ClientPacketProcessor::~ClientPacketProcessor()
{
}

void ClientPacketProcessor::recieveMapGen(ENetPacket * pkt)
{
	if (mazeNode == nullptr) {
		mazeNode = new RenderNode();
	}

	if (pkt->dataLength == sizeof(WallDescriptor)) {
		WallDescriptor w;
		memcpy(&w, pkt->data, sizeof(WallDescriptor));
		wds.push_back(w);

		Vector3 start = Vector3(
			float(w._xs),
			0.0f,
			float(w._ys));

		Vector3 size = Vector3(
			float(w._xe - w._xs),
			0.0f,
			float(w._ye - w._ys)
		);


		const float scalar = 1.f / (float)(u_int(mazeSize) * 3 - 1);

		start = start * scalar;
		Vector3 end = start + size * scalar;
		end.y = 0.75f;

		Vector3 centre = (end + start) * 0.5f;
		
		Vector3 halfDims = (centre - start);
		halfDims.y = 0.18;

		RenderNode* cube = new RenderNode(wallmesh, Vector4(1,0,0,1));
		
		cube->SetTransform(Matrix4::Translation(centre * Vector3(10,0,10)) * Matrix4::Scale(halfDims * Vector3(10,1,10)));
		mazeNode->AddChild(cube);
		//scene->AddGameObject(new GameObject("wall",cube));



	}
	else {
		currentProc = processType::WAITING;
	}
	
}

void ClientPacketProcessor::buildPath(ENetPacket * pkt)
{
	if (pkt->dataLength == sizeof(Vector3)) {
		Vector3 newNode;
		memcpy(&newNode, pkt->data, sizeof(Vector3));
		//newNode = newNode * Vector3(5,1,5);
		//newNode += Vector3(0, 2, 0);

		path.push_back(newNode);
	}
	else {
		currentProc = processType::WAITING;
	}
	
}

void ClientPacketProcessor::buildMapWalls()
{
	
	const float scalar = 1.f / (float)(u_int(mazeSize) * 3 - 1);
	RenderNode* cube = new RenderNode(wallmesh, Vector4(1,0,0,1));
	cube->SetTransform(Matrix4::Translation(Vector3(-scalar*0.5f, 0.0f, 0.5)*10) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f) * Vector3(10,1,10)));
	mazeNode->AddChild(cube);
	

	cube = new RenderNode(wallmesh, Vector4(1, 0, 0, 1));
	cube->SetTransform(Matrix4::Translation(Vector3(1.f + scalar*0.5f, 0.0f, 0.5)*10) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f) * Vector3(10,1,10)));
	mazeNode->AddChild(cube);
	

	cube = new RenderNode(wallmesh, Vector4(1, 0, 0, 1));
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.0f, -scalar*0.5f)*10) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f) * Vector3(10,1,10)));
	mazeNode->AddChild(cube);
	

	cube = new RenderNode(wallmesh, Vector4(1, 0, 0, 1));
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.0f, 1.f + scalar*0.5f)*10) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f) * Vector3(10,1,10)));
	mazeNode->AddChild(cube);

	currentProc = processType::WAITING;
	scene->AddGameObject(new GameObject("maze", mazeNode));
}

void ClientPacketProcessor::sendStartPos(int start)
{
	//send flag to server processor
	int startFlag = 1;
	ENetPacket* packet = enet_packet_create(&startFlag, sizeof(int),0);
	enet_peer_send(serverConnection, 0, packet);


	packet = enet_packet_create(&start, sizeof(int), 0);
	//connection, flags, packet
	enet_peer_send(serverConnection, 0, packet);
}

void ClientPacketProcessor::sendGoalPos(int end)
{
	//send flag to server processor
	int startFlag = 2;
	ENetPacket* packet = enet_packet_create(&startFlag, sizeof(int), 0);
	enet_peer_send(serverConnection, 0, packet);


	packet = enet_packet_create(&end, sizeof(int), 0);
	//connection, flags, packet
	enet_peer_send(serverConnection, 0, packet);
}

void ClientPacketProcessor::sendMapGen(int dimen)
{
	//send flag first
	int flag = 0;
	//pointer to data, size of packet, flags
	ENetPacket* packet = enet_packet_create(&flag, sizeof(int), 0);
	//connection, flags, packet
	enet_peer_send(serverConnection, 0, packet);

	//pointer to data, size of packet, flags
	 packet = enet_packet_create(&dimen, sizeof(int), 0);
	//connection, flags, packet
	enet_peer_send(serverConnection, 0, packet);
}

void ClientPacketProcessor::spawnAvatar(int startPos)
{
	int spawnFlag = 3;
	ENetPacket* packet = enet_packet_create(&spawnFlag, sizeof(int), 0);
	enet_peer_send(serverConnection, 0, packet);

	int initFlag = 0;
	packet = enet_packet_create(&spawnFlag, sizeof(int), 0);
	enet_peer_send(serverConnection, 0, packet);

	if (!scene->FindGameObject("Avatar")) {
		GameObject* avatar = CommonUtils::BuildCuboidObject(
			"Avatar",
			Vector3(0.0f, 1.0f, 0.0f),
			Vector3(0.5f, 0.5f, 0.5f),
			true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		scene->AddGameObject(avatar);
	}
}

void ClientPacketProcessor::processPacket(ENetPacket * pkt)
{
	switch (currentProc) {
	case(processType::MAPGEN) :
		recieveMapGen(pkt);
		break;
	case(processType::MAPFINISH) :
		buildMapWalls();
		break;
	case(processType::PATHDETAILS) :
		buildPath(pkt);
		break;
	case(processType::AVATARUPDATE) :
		updateAvatar(pkt);
		break;
	case(processType::RAPTORUPDATE) :
		updateRaptors(pkt);
		break;
	case(processType::WAITING) :
		determinePacketInst(pkt);
		break;
	}
	
}

void ClientPacketProcessor::determinePacketInst(ENetPacket * pkt)
{
	int pktData;
	memcpy(&pktData,pkt->data, sizeof(int));
	

	if (pktData == processType::PATHDETAILS) {
		//delete current path details first
		path.clear();
		currentProc = processType::PATHDETAILS;
	}
	if (pktData == processType::MAPGEN) {
		currentProc = processType::MAPGEN;
	}
	 if (pktData == processType::MAPFINISH) {
		currentProc = processType::MAPFINISH;
		processPacket(pkt);
	}
	 if (pktData == processType::AVATARUPDATE) {
		 currentProc = processType::AVATARUPDATE;
	 }
	 if (pktData == processType::RAPTORUPDATE) {
		 currentProc = processType::RAPTORUPDATE;
	 }
}

void ClientPacketProcessor::updateAvatar(ENetPacket * pkt)
{
	Vector3 updatedPos;
	memcpy(&updatedPos, pkt->data, sizeof(Vector3));
	std::cout << updatedPos << "\n";
	if (scene->FindGameObject("Avatar")) {
		scene->FindGameObject("Avatar")->Physics()->SetPosition(updatedPos);
		enet_packet_destroy(pkt);
	}
	currentProc = processType::WAITING;
}

void ClientPacketProcessor::updateRaptors(ENetPacket * pkt)
{
	if (pkt->dataLength == sizeof(Vector3)) {
		if (scene->FindGameObject("Raptor" + raptorCount)) {
			Vector3 updatedPos;
			memcpy(&updatedPos, pkt->data, sizeof(Vector3));
			scene->FindGameObject("Raptor" + raptorCount)->Physics()->SetPosition(updatedPos);
		}
		else {
			GameObject* raptor = CommonUtils::BuildCuboidObject(
				"Raptor" + raptorCount,
				Vector3(0.0f, 1.0f, 0.0f),
				Vector3(0.5f, 0.5f, 0.5f),
				true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
				0.0f,
				false,
				false,
				Vector4(0.8f, 0.5f, 0.1f, 1.0f));
			scene->AddGameObject(raptor);
		}
		raptorCount++;
	}
	else {
		currentProc = processType::WAITING;
		raptorCount = 0;
	}


}
