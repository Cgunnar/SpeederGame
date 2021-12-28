#pragma once


#include "RimfrostMath.hpp"
#include "StandardComponents.h"
#include "RenderComponents.h"
#include "GuiDebug.h"
#include "SkyBox.h"

class Scene
{
public:
	Scene();
	~Scene();

	void Update(float dt);
	rfe::Entity& GetCamera();

	SkyBox sky;
private:
	rfe::Entity m_camera;
	rfe::Entity m_nanosuit;
	rfe::Entity m_pistol;
	rfe::Entity m_ship;
	rfe::Entity m_arrow;
	rfe::Entity m_quad;
	rfe::Entity m_ironSphere;
	rfe::Entity m_brickWallFloor;
	rfe::Entity m_pointLight;
	GuiTest m_quadContr = GuiTest("quad controller");
	GuiTest m_lightContr = GuiTest("pointLight controller");

};

