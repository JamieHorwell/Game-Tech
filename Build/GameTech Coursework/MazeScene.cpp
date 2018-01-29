#include "MazeScene.h"
#include "../ncltech/CommonUtils.h"
#include "../nclgl/OBJMesh.h"




MazeScene::MazeScene(const std::string & friendly_name) : Scene(friendly_name)
, m_AccumTime(0.0f) , serverConnection(NULL), pktProcessor(this)
{
	wallmesh = new OBJMesh(MESHDIR"cube.obj");

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallmesh->SetTexture(whitetex);

	srand(93225); //Set the maze seed to a nice consistent example :)
	pktProcessor.setWallMesh(wallmesh);
}

MazeScene::~MazeScene()
{
	SAFE_DELETE(wallmesh);
}

void MazeScene::OnInitializeScene()
{
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		pktProcessor.setServerConn(serverConnection);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}



	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(-1.5, 25, 1));
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(-80);
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(0);



	GenerateGround();

	
}

void MazeScene::OnCleanupScene()
{
	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;

}

void MazeScene::OnUpdateScene(float dt)
{

	//Update Network
	auto callback = std::bind(
		&MazeScene::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);



	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		if (this->FindGameObject("maze")) { delete this->FindGameObject("maze"); };
		pktProcessor.deleteMaze();
		//send test map generate
		int* dimensions;
		if (mazeDimen > 1) {
			mazeDimen = mazeDimen - 1;
			dimensions = new int(mazeDimen);
		}
		else {
			dimensions = new int(1);
		}
		pktProcessor.sendMapGen(*dimensions);
		pktProcessor.setMazeSize(*dimensions);
		delete dimensions;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
		//pktProcessor.deleteMaze();
		if (this->FindGameObject("maze")) { delete this->FindGameObject("maze"); };
		pktProcessor.deleteMaze();
		int* dimensions;
		if (mazeDimen < 15) {
			mazeDimen = mazeDimen + 1;
			dimensions = new int(mazeDimen);
			
		}
		else {
			dimensions = new int(mazeDimen);
		}
		pktProcessor.sendMapGen(*dimensions);
		pktProcessor.setMazeSize(*dimensions);
		delete dimensions;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
		if (startPos > 0) {
			startPos--;
		}
		pktProcessor.sendStartPos(startPos);
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
		if (endPos < mazeDimen * mazeDimen) {
			endPos++;
		}
		pktProcessor.sendGoalPos(endPos);
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5)) {
		pktProcessor.spawnAvatar(startPos);
	}
	debugDrawPath();

	NCLDebug::AddStatusEntry(Vector4(1, 0, 1, 1), "Network Traffic");
	NCLDebug::AddStatusEntry(Vector4(1, 0, 1, 1), "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(Vector4(1, 0, 1, 1), "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
}

void MazeScene::GenerateGround()
{

	GameObject* ground = CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		false,
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));

	this->AddGameObject(ground);
}

void MazeScene::ProcessNetworkEvent(const ENetEvent & evnt)
{
	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == serverConnection)
		{
			
		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{

		pktProcessor.processPacket(evnt.packet);
	}
	break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		
	}
	break;
	}
}

void MazeScene::debugDrawPath()
{
	float grid_scalar = 1.0f / (float)u_int(pktProcessor.getMazeSize());

	if (pktProcessor.getPath().size() > 0) {
		for (int i = 0; i < pktProcessor.getPath().size() - 1; ++i) {

			Vector3 start = Vector3(
				(pktProcessor.getPath()[i].x * 10 + 0.5f * 10) * grid_scalar,
				0.1f,
				(pktProcessor.getPath()[i].z * 10 + 0.5f * 10) * grid_scalar);

			Vector3 end = Vector3(
				(pktProcessor.getPath()[i+1].x * 10 + 0.5f * 10) * grid_scalar,
				0.1f,
				(pktProcessor.getPath()[i+1].z * 10 + 0.5f * 10) * grid_scalar);


			NCLDebug::DrawThickLine(start, end, 0.1, Vector4(1, 0, 1, 1));

		}
	}
	
}


