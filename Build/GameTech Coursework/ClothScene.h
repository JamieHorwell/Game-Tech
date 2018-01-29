#pragma once
#include <ncltech\Scene.h>
#include <ncltech\Cloth.h>
#include <ncltech\Hull.h>
#include <ncltech\SpringConstraint.h>


class ClothScene : public Scene
{
public:
	ClothScene(const std::string& friendly_name);
	virtual ~ClothScene();

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;

protected:
	float m_AccumTime;
	Cloth* cloth;
	GameObject* handle;
	GameObject* ball;

};

