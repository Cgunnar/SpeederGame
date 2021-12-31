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
	rfe::Entity CreateEntityModel(const std::string path, rfm::Vector3 pos = { 0,0,0 }, rfm::Vector3 rotDeg = { 0,0,0 }, rfm::Vector3 scale = { 1,1,1 });

	rfe::Entity m_camera;
	rfe::Entity m_ship;
	rfe::Entity m_arrow;
	rfe::Entity m_quad;
	rfe::Entity m_ironSphere;
	rfe::Entity m_debugSphere;
	rfe::Entity m_pointLight;
	rfe::Entity m_dirLight;
	rfe::Entity m_terrain;

	std::vector<rfe::Entity> m_entities;

	GuiTest m_quadContr = GuiTest("quad controller");
	GuiTest m_lightContr = GuiTest("pointLight controller");
	GuiTest m_dirlightContr = GuiTest("dirLight controller");

};

