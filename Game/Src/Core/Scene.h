#pragma once


#include "RimfrostMath.hpp"
#include "StandardComponents.h"
#include "RenderComponents.h"
#include "GuiDebug.h"

class Scene
{
public:
	Scene();
	~Scene();

	void Update(float dt);
	rfe::Entity& GetCamera();

private:
	rfe::Entity m_camera;
	rfe::Entity m_sun;
	rfe::Entity m_quad;
	rfe::Entity m_pointLight;
	GuiTest m_quadContr = GuiTest("quad controller");
	GuiTest m_lightContr = GuiTest("pointLight controller");

};

