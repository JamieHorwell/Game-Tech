#include "ClothScene.h"
#include <ncltech\CommonUtils.h>





ClothScene::ClothScene(const std::string & friendly_name) : Scene(friendly_name)
, m_AccumTime(0.0f)
{
}

ClothScene::~ClothScene()
{
}

void ClothScene::OnInitializeScene()
{
	cloth = new Cloth(10, 10, Vector3(0,5,0));
	PhysicsEngine::Instance()->setCloth(cloth);
	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
	//drawFlags ^= DEBUGDRAW_FLAGS_CONSTRAINT;
	PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

	PhysicsEngine::Instance()->getBPcull().init(Vector3(-20,-1,-20),Vector3(20,20,20));






	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.5f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		false,
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


	//Create Hanging Ball
	handle = CommonUtils::BuildSphereObject("",
		Vector3(-7.f, 7.f, -5.0f),				//Position
		0.5f,									//Radius
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		false,									//No Collision Shape Yet
		true,									//Dragable by the user
		CommonUtils::GenColor(0.45f, 0.5f));	//Color

	ball = CommonUtils::BuildSphereObject("",
		Vector3(-7.f, 5.f, -5.0f),				//Position
		0.5f,									//Radius
		true,									//Has Physics Object
		1.0f,									// Inverse Mass = 1 / 1kg mass
		false,									//No Collision Shape Yet
		true,									//Dragable by the user
		CommonUtils::GenColor(0.5f, 1.0f));		//Color

	this->AddGameObject(handle);
	this->AddGameObject(ball);

	//Add distance constraint between the two objects
	SpringConstraint* constraint = new SpringConstraint(handle->Physics(),ball->Physics());
	PhysicsEngine::Instance()->AddConstraint(constraint);
}

void ClothScene::OnCleanupScene()
{
	delete cloth;
	PhysicsEngine::Instance()->setCloth(nullptr);
}

void ClothScene::OnUpdateScene(float dt)
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
		if (this->FindGameObject("playerSphere")) {
			this->RemoveGameObject(this->FindGameObject("playerSphere"));
		}
		Vector3 pos = GraphicsPipeline::Instance()->GetCamera()->GetPosition();
		GameObject* sphere = CommonUtils::BuildSphereObject(
			"playerSphere",					// Optional: Name
			pos,				// Position
			1,			// Half-Dimensions
			true,				// Physics Enabled?
			0.1f,				// Physical Mass (must have physics enabled)
			true,				// Physically Collidable (has collision shape)
			false,				// Dragable by user?
			Vector4(1, 0, 0, 1));// Render color


		this->AddGameObject(sphere);
		Matrix3 camMat = GraphicsPipeline::Instance()->GetCamera()->BuildViewMatrix();
		Vector3 dir = -camMat.GetRow(2);
		this->FindGameObject("playerSphere")->Physics()->SetLinearVelocity(dir * 50);
	}
	cloth->DebugDraw();

	
}
