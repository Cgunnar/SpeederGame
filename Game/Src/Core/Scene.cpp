#include "pch.hpp"
#include "Scene.h"
#include "AssetManager.h"
#include "Geometry.h"
#include "LowLvlGfx.h"
#include "ReadImg.hpp"
#include "AssimpLoader.h"
#include "GraphicsHelperFunctions.h"
#include "GraphicsResources.h"
#include "CameraControllerScript.h"
#include "ShipScript.h"
#include "Input.h"
#include "TerrainMeshGenerator.h"
#include "TerrainScript.h"
#include "TerrainMapGenerator.h"
#include "PhysicsComponents.h"
#include "FrameTimer.hpp"
#include "imgui.h"

using namespace rfm;
using namespace rfe;


Scene::Scene()
{
	AssetManager& am = AssetManager::Get();
	
	m_terrDesc.lacunarity = 2;
	m_terrDesc.octaves = 10;
	m_terrDesc.persistence = 0.5f;
	m_terrDesc.frequencyScale = 150;
	m_terrDesc.baseOffset = {0,0};
	m_terrDesc.seed = 10;
	m_terrDesc.heightScale = 100;
	//m_terrDesc.heightScaleFunc = [](float h) {return h < 0.4f ? 0.0f : 1.0f - sqrt(1.0f - (h-0.4f)*(h-0.4f)); };
	/*m_terrDesc.bioms.emplace_back("water", Vector3(0,0,1), 0, !true);
	m_terrDesc.bioms.emplace_back("grassLand", Vector3(0,1,0), 0.3f);
	m_terrDesc.bioms.emplace_back("mountain", 0.2f, 0.55f);
	m_terrDesc.bioms.emplace_back("mountain_snow", 0.9f, 0.94f);*/
	m_terrDesc.bioms.emplace_back("mountain", Vector3(0.2f, 0.2f, 0.2f), 0.0f);
	m_terrDesc.LODs.push_back({ .lod = 0, .visDistThrhold = 200 });
	m_terrDesc.LODs.push_back({ .lod = 1, .visDistThrhold = 400 });
	m_terrDesc.LODs.push_back({ .lod = 2, .visDistThrhold = 600 });
	m_terrDesc.LODs.push_back({ .lod = 3, .visDistThrhold = 800 });

	m_terrain = EntityReg::CreateEntity();
	m_terrain.AddComponent<TransformComp>();
	m_terrain.GetComponent<TransformComp>()->transform.setScale(0.6f);
	m_terrain.GetComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);
	m_terrain.GetComponent<TransformComp>()->transform.setTranslation(0, -30, 0);
	m_terrain.AddComponent<TerrainScript>(m_terrDesc);


	CreateEntityModel("Assets/Models/brick_wall/brick_wall.obj", {0, -1, 0}, { 90, 0, 0 }, 10);


	m_ship = CreateEntityModel("Assets/Models/pbr/ajf-12_dvergr/scene.gltf", { 0, 10, 3 });
	m_ship.AddComponent<ShipScript>();
	m_ship.AddComponent<PlayerComp>();

	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 10, -4);
	m_camera.AddComponent<CameraControllerScript>();
	m_camera.AddComponent<CameraComp>();

	
	TerrainMeshGenerator tl;
	TerrainMeshDesc td;
	td.uvScale = 1;
	auto terMesh = tl.CreateTerrainMeshFromBMP("Assets/Textures/noiseTexture.bmp", td);
	Mesh terrainMesh3(terMesh.verticesTBN, terMesh.indices);

	Material terrainMatSand;
	terrainMatSand.name = "terrainSandMaterial";
	terrainMatSand.emissiveFactor = 0;
	terrainMatSand.SetMetallicRoughnessTexture("Assets/Textures/sand/metallic_roughness.png");
	terrainMatSand.SetBaseColorTexture("Assets/Textures/sand/basecolor.jpg");
	terrainMatSand.SetNormalTexture("Assets/Textures/sand/normal.jpg");
	terrainMatSand.flags |= RenderFlag::sampler_anisotropic_wrap;
	m_oldTerrain = EntityReg::CreateEntity();
	m_oldTerrain.AddComponent<TransformComp>()->transform.setTranslation({ 100, 0, 0 });
	
	
	m_oldTerrain.AddComponent<RenderModelComp>(am.AddRenderUnit(terrainMesh3, terrainMatSand));


	sky.Init("Assets/Textures/MonValley_Lookout/MonValley_A_LookoutPoint_2k.hdr");


	//CreateEntityModel("Assets/Models/MetalRoughSpheres/glTF/pbrSpheres.gltf", { -2, 3, 4 }, { 0, 0, 0 }, 0.2f);
	//CreateEntityModel("Assets/Models/pbr/razor_crest/scene.gltf", { 4, 5, 0 }, { 0, 0, 0 }, 0.2f);
	/*CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 4, 2, 2 }, 0, 0.03f);
	CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 3, 2, 4 }, { 0,-70, 0 }, 0.03f);
	CreateEntityModel("Assets/Models/nanosuit/nanosuit.obj", { 2, 1, 5 }, 0, 0.1f);*/


	m_arrow = EntityReg::CreateEntity();
	m_arrow.AddComponent(TransformComp());
	m_arrow.AddComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj"));

	m_pointLight = EntityReg::CreateEntity();
	m_pointLight.AddComponent<TransformComp>();
	m_pointLight.AddComponent<PointLightComp>()->pointLight.lightStrength = 5;

	sunLight = EntityReg::CreateEntity();
	sunLight.AddComponent<TransformComp>();
	sunLight.AddComponent<DirectionalLightComp>()->dirLight.color = { 1, 0.87f, 0.23f };

	
	m_plane = EntityReg::CreateEntity();
	m_plane.AddComponent<TransformComp>()->transform.setRotationDeg(90, 0, 0);
	m_plane.GetComponent<TransformComp>()->transform.setScale(10);
	Material planeMat;
	planeMat.baseColorFactor = Vector4(1, 1, 1, 0.8f);
	planeMat.flags |= RenderFlag::alphaBlend;
	m_plane.AddComponent<RenderUnitComp>(SimpleMesh::Quad_POS_NOR_UV, planeMat);


	Material rusteIronMat;
	rusteIronMat.SetBaseColorTexture("Assets/Textures/rustediron/basecolor.png");
	rusteIronMat.SetMetallicRoughnessTexture("Assets/Textures/rustediron/metallic_roughness.png");
	rusteIronMat.SetNormalTexture("Assets/Textures/rustediron/normal.png");
	rusteIronMat.metallicFactor = 1;
	rusteIronMat.roughnessFactor = 1;

	m_ironSphere = EntityReg::CreateEntity();
	m_ironSphere.AddComponent(TransformComp())->transform.setTranslation(-4, 3, 1);
	m_ironSphere.AddComponent<RenderModelComp>(AssetManager::Get().AddRenderUnit(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN, rusteIronMat));

	/*float debugSphereScale = 0.1f;
	Material debugSphereMat;
	debugSphereMat.baseColorFactor = 0;
	debugSphereMat.emissiveFactor = Vector3(1, 0, 0);
	m_sphere0 = EntityReg::CreateEntity();
	m_sphere0.AddComponent<TransformComp>()->transform.setScale(debugSphereScale);
	m_sphere0.AddComponent<RenderUnitComp>(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN, debugSphereMat);

	debugSphereMat.emissiveFactor = Vector3(0, 1, 0);
	m_sphere1 = EntityReg::CreateEntity();
	m_sphere1.AddComponent<TransformComp>()->transform.setScale(debugSphereScale);
	m_sphere1.AddComponent<RenderUnitComp>(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN, debugSphereMat);

	debugSphereMat.emissiveFactor = Vector3(0, 0, 1);
	m_sphere2 = EntityReg::CreateEntity();
	m_sphere2.AddComponent<TransformComp>()->transform.setScale(debugSphereScale);
	m_sphere2.AddComponent<RenderUnitComp>(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN, debugSphereMat);*/

	/*debugSphereMat.emissiveFactor = Vector3(1, 1, 0);
	m_sphere3 = EntityReg::CreateEntity();
	m_sphere3.AddComponent<TransformComp>()->transform.setScale(0.1f);
	m_sphere3.AddComponent<RenderUnitComp>(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN, debugSphereMat);*/


	m_dirlightContr.slider1.ChangeDefaultValues({ -0.922f ,-0.176f, 1}, -1, 1);
	m_dirlightContr.slider2.ChangeDefaultValues(sunLight.GetComponent<DirectionalLightComp>()->dirLight.color, 0, 1);

	FrameTimer::NewFrame();
	EntityReg::StartScripts<CameraControllerScript, ShipScript, TerrainScript>();
}

Scene::~Scene()
{
	
}

void Scene::Update(float dt)
{
	EntityReg::RunScripts<CameraControllerScript, ShipScript, TerrainScript>(dt);

	/*if (m_terrainGUI.Show())
	{
		if (m_terrain.GetComponent<TerrainScript>())
		{
			m_terrain.RemoveComponent<TerrainScript>();
		}
		
		auto guiInput = m_terrainGUI.GetValues();
		m_terrDesc.baseOffset = guiInput.baseOffset;
		m_terrDesc.frequencyScale = guiInput.frequencyScale;
		m_terrDesc.lacunarity = guiInput.lacunarity;
		m_terrDesc.octaves = guiInput.octaves;
		m_terrDesc.persistence = guiInput.persistence;
		m_terrDesc.heightScale = guiInput.heightScale;
		m_terrDesc.seed = guiInput.seed;
		m_terrain.AddComponent<TerrainScript>(m_terrDesc);
		m_terrain.GetComponent<TransformComp>()->transform.setScale(guiInput.scale);
	}*/

	static bool shipCam = false;
	if (Input::Get().keyPressed(Input::Y)) shipCam = !shipCam;
	if (shipCam) m_camera.GetComponent<TransformComp>()->transform = m_ship.GetComponent<ShipScript>()->GetCameraFollowTransform();;

	m_dirlightContr.Show();

	sunLight.GetComponent<DirectionalLightComp>()->dirLight.dir = m_dirlightContr.slider1.value;
	sunLight.GetComponent<DirectionalLightComp>()->dirLight.color = m_dirlightContr.slider2.value;

}

rfe::Entity& Scene::GetCamera()
{
	return m_camera;
}

rfe::Entity Scene::CreateEntityModel(const std::string path, Vector3 pos, Vector3 rotDeg, Vector3 scale)
{
	m_entities.push_back(EntityReg::CreateEntity());
	Transform& t = m_entities.back().AddComponent<TransformComp>()->transform;
	t.setTranslation(pos);
	t.setRotationDeg(rotDeg.x, rotDeg.y, rotDeg.z);
	t.setScale(scale.x, scale.y, scale.z);

	m_entities.back().AddComponent<RenderModelComp>(path);
	return m_entities.back();
}
