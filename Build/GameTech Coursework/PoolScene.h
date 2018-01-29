#pragma once

#include <ncltech\Scene.h>

class PoolScene  : public Scene
{
public:
	PoolScene(const std::string& friendly_name);
	virtual ~PoolScene();

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;

protected:
	float m_AccumTime;

};

