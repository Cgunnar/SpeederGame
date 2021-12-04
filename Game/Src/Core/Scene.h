#pragma once


#include "RimfrostMath.hpp"
#include "StandardComponents.h"
#include "RenderComponents.h"

class Scene
{
public:
	Scene();
	~Scene();

	void Update(float dt);
	rfe::Entity& GetCamera();

private:
	rfe::Entity m_camera;
	rfe::Entity m_quad;

};

