#pragma once


#include "RimfrostMath.hpp"
#include "StandardComponents.h"
#include "RenderComponents.h"
#include "GuiDebug.h"
#include "TerrainGUI.h"
#include "TerrainTypes.h"
#include "SkyBox.h"
#include "EnvironmentMap.h"

class Scene
{
public:
	Scene();
	~Scene();

	void Update(float dt);
	rfe::Entity& GetCamera();
	std::unique_ptr<EnvironmentMap>& GetEnvMap();
	SkyBox sky;
	rfe::Entity sunLight;
private:
	rfe::Entity CreateEntityModel(const std::string path, rfm::Vector3 pos = { 0,0,0 }, rfm::Vector3 rotDeg = { 0,0,0 }, rfm::Vector3 scale = { 1,1,1 });
	TerrainDesc m_terrDesc;
	rfe::Entity m_camera;
	rfe::Entity m_ship;
	rfe::Entity m_arrow;
	rfe::Entity m_terrain;
	rfe::Entity m_ironSphere;
	rfe::Entity m_sphere0;
	rfe::Entity m_sphere1;
	rfe::Entity m_sphere2;
	rfe::Entity m_sphere3;
	rfe::Entity m_pointLight;

	std::vector<rfe::Entity> m_entities;

	std::unique_ptr<EnvironmentMap> m_envMap = nullptr;

	GuiTest m_dirlightContr = GuiTest("dirLight controller");
	TerrainGUI m_terrainGUI = TerrainGUI("terrain generation");
};

