#include "PoolScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

using namespace CommonUtils;

PoolScene::PoolScene(const std::string& friendly_name) : Scene(friendly_name)
,m_AccumTime(0.0f)
{

}


PoolScene::~PoolScene()
{
}

void PoolScene::OnInitializeScene()
{
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(true);

	//Set the camera position
	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(140.f);
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(-20.f);


	//SCENE CREATION
	PhysicsEngine::Instance()->getBPcull().init(Vector3(-20,-10,-20),Vector3(20,10,20));
	this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));



	//create our pool edges
	this->AddGameObject(BuildCuboidObject("poolSideLeft", Vector3(5.0f, 0.0f, 0.0f), Vector3(2.0f, 0.5f, 10.0f), true, 0.0f, true, false, Vector4(0.5f, 0.3f, 0.8f, 1.0f)));
	this->FindGameObject("poolSideLeft")->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 90));

	this->AddGameObject(BuildCuboidObject("poolSideRight", Vector3(-5.0f, 0.0f, 0.0f), Vector3(2.0f, 0.5f, 10.0f), true, 0.0f, true, false, Vector4(0.5f, 0.3f, 0.8f, 1.0f)));
	this->FindGameObject("poolSideRight")->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 90));

	this->AddGameObject(BuildCuboidObject("poolSideTop", Vector3(0.0f, 0.0f, -10.0f), Vector3(5.0f, 2.0f, 0.5f), true, 0.0f, true, false, Vector4(0.5f, 0.3f, 0.8f, 1.0f)));
	
	this->AddGameObject(BuildCuboidObject("poolSideBottom", Vector3(0.0f, 0.0f, 10.0f), Vector3(5.0f, 2.0f, 0.5f), true, 0.0f, true, false, Vector4(0.5f, 0.3f, 0.8f, 1.0f)));

	//create balls for pool
	auto create_ball_cube = [&](const Vector3& offset, const Vector3& scale, float ballsize)
	{
		const int dims = 5;
		const Vector4 col = Vector4(1.0f, 0.5f, 0.2f, 1.0f);

		for (int x = 0; x < dims; ++x)
		{
			for (int y = 0; y < dims; ++y)
			{
				for (int z = 0; z < dims; ++z)
				{
					Vector3 pos = offset + Vector3(scale.x *x, scale.y * y, scale.z * z);
					GameObject* sphere = BuildSphereObject(
						"",					// Optional: Name
						pos,				// Position
						ballsize,			// Half-Dimensions
						true,				// Physics Enabled?
						10.f,				// Physical Mass (must have physics enabled)
						true,				// Physically Collidable (has collision shape)
						true,				// Dragable by user?
						col);// Render color
					this->AddGameObject(sphere);
				}
			}
		}
	};


	auto create_cloth = [&](const Vector3& position, int xDimensions, int yDimensions) {
		for (int x = 0; x < xDimensions; ++x) {
			for (int y = 0; y < yDimensions; ++y) {
				Vector3 pos = position + Vector3(xDimensions,-yDimensions,0);

				GameObject* sphere = BuildSphereObject(
					"cloth" + xDimensions + yDimensions,					// Optional: Name
					pos,				// Position
					0.5,			// Half-Dimensions
					true,				// Physics Enabled?
					10.f,				// Physical Mass (must have physics enabled)
					true,				// Physically Collidable (has collision shape)
					true,				// Dragable by user?
					Vector4(0,0,0,0));// Render color
				this->AddGameObject(sphere);


			}
		}

		//now add constraints
		for (int x = 0; x < xDimensions; ++x) {
			for (int y = 0; y < yDimensions; ++y) {

			}
		}
	};


	
				create_ball_cube(Vector3(0,10,0),Vector3(0.5,0.5,0.5),0.3);
		

}

void PoolScene::OnCleanupScene() {
	Scene::OnCleanupScene();
}

void PoolScene::OnUpdateScene(float dt) {
	m_AccumTime += dt;


}