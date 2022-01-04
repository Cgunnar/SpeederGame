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
#include "ShipContollerScript.h"
#include "Input.h"
#include "TerrainMeshGenerator.h"
#include "TerrainScript.h"
#include "TerrainGenerator.h"

using namespace rfm;
using namespace rfe;


Scene::Scene()
{
	AssetManager& am = AssetManager::Get();

	TerrainMapDesc terrDesc;
	terrDesc.lacunarity = 2;
	terrDesc.octaves = 4;
	terrDesc.persistence = 0.5f;
	terrDesc.scale = 27.6f;
	terrDesc.offset = { 4, 12 };
	terrDesc.seed = 32;

	terrDesc.bioms.emplace_back("water", Vector3(0,0,1), 0.3f, true);
	terrDesc.bioms.emplace_back("grassLand", Vector3(0,1,0), 0.5);
	terrDesc.bioms.emplace_back("mountain", 0.2f, 1);

	auto f = TerrainGenerator::GenerateTerrinMap(terrDesc);


	TerrainMeshGenerator t2;
	t2.CreateTerrainMesh(f, 10, 0, 0, [](float in) {return in <= 0.3f ? 0.3f*0.3f : in * in; });
	SubMesh terrainMesh2(t2.GetVerticesTBN(), t2.GetIndices());


	Material terrainMat;
	terrainMat.baseColorTexture = am.LoadTex2DFromMemoryR8G8B8A8(f.colorMapRGBA.data(), f.width, f.height, LoadTexFlag::GenerateMips);
	terrainMat.emissiveFactor = 0;
	m_terrain = EntityReg::CreateEntity();
	m_terrain.AddComponent<TransformComp>()->transform.setScale(0.01f);
	m_terrain.GetComponent<TransformComp>()->transform.setTranslation(0, 1, 0);
	m_terrain.AddComponent<RenderModelComp>(AssetManager::Get().AddRenderUnit(terrainMesh2, terrainMat));
	m_terrain.AddComponent<TerrainScript>();





	CreateEntityModel("Assets/Models/brick_wall/brick_wall.obj", {0, -1, 0}, { 90, 0, 0 }, 10);
	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 2, -4);
	m_camera.AddComponent<CameraControllerScript>();
	m_camera.AddComponent<PlayerComp>(); // for now the camera is the player

	
	TerrainMeshGenerator tl;
	tl.CreateTerrainMeshFromBMP("Assets/Textures/noiseTexture.bmp", 10, 0);
	SubMesh terrainMesh(tl.GetVerticesTBN(), tl.GetIndices());

	Material terrainMatSand;
	terrainMatSand.name = "terrainMaterial";
	terrainMatSand.emissiveFactor = 0;
	terrainMatSand.SetMetallicRoughnessTexture("Assets/Textures/sand/metallic_roughness.png");
	terrainMatSand.SetBaseColorTexture("Assets/Textures/sand/basecolor.jpg");
	terrainMatSand.SetNormalTexture("Assets/Textures/sand/normal.jpg");

	m_oldTerrain = EntityReg::CreateEntity();
	m_oldTerrain.AddComponent<TransformComp>()->transform.setTranslation({ -64, -9, -64 });
	
	
	m_oldTerrain.AddComponent<RenderModelComp>(am.AddRenderUnit(terrainMesh, terrainMatSand));


	sky.Init("Assets/Textures/MonValley_Lookout/MonValley_A_LookoutPoint_2k.hdr");


	//CreateEntityModel("Assets/Models/MetalRoughSpheres/glTF/pbrSpheres.gltf", { -2, 3, 4 }, { 0, 0, 0 }, 0.2f);
	//CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 4, 2, 2 }, 0, 0.03f);
	//CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 3, 2, 4 }, { 0,-70, 0 }, 0.03f);
	//CreateEntityModel("Assets/Models/nanosuit/nanosuit.obj", { 2, 1, 5 }, 0, 0.1f);


	m_arrow = EntityReg::CreateEntity();
	m_arrow.AddComponent(TransformComp());
	m_arrow.AddComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj"));

	m_ship = CreateEntityModel("Assets/Models/pbr/ajf-12_dvergr/scene.gltf", { 0, 2, 3 });
	m_ship.AddComponent<ShipContollerScript>();

	

	m_pointLight = EntityReg::CreateEntity();
	m_pointLight.AddComponent<TransformComp>();
	m_pointLight.AddComponent<PointLightComp>()->pointLight.lightStrength = 5;

	sunLight = EntityReg::CreateEntity();
	sunLight.AddComponent<TransformComp>();
	sunLight.AddComponent<DirectionalLightComp>()->dirLight.color = { 1, 0.87f, 0.23f };

	



	Material rusteIronMat;
	rusteIronMat.SetBaseColorTexture("Assets/Textures/rustediron/basecolor.png");
	rusteIronMat.SetMetallicRoughnessTexture("Assets/Textures/rustediron/metallic_roughness.png");
	rusteIronMat.SetNormalTexture("Assets/Textures/rustediron/normal.png");

	SubMesh quadMeshCopy2 = AssetManager::Get().GetMesh(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN);
	m_ironSphere = EntityReg::CreateEntity();
	m_ironSphere.AddComponent(TransformComp())->transform.setTranslation(0, 3, 1);
	m_ironSphere.AddComponent(RenderModelComp(AssetManager::Get().AddRenderUnit(quadMeshCopy2, rusteIronMat)));


	m_quadContr.slider1.ChangeDefaultValues({ 0,1,-1.2f });

	m_lightContr.slider1.ChangeDefaultValues({ 0,2,-9 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);

	m_dirlightContr.slider1.ChangeDefaultValues({ -0.922f ,-0.176f, 1}, -1, 1);
	m_dirlightContr.slider2.ChangeDefaultValues(sunLight.GetComponent<DirectionalLightComp>()->dirLight.color, 0, 1);

	EntityReg::StartScripts<CameraControllerScript, ShipContollerScript, TerrainScript>();
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{

	



	EntityReg::RunScripts<CameraControllerScript, ShipContollerScript, TerrainScript>(dt);

	Transform followShip = m_ship.GetComponent<TransformComp>()->transform;
	followShip.translateL(0, 1, -4);
	//m_camera.getComponent<TransformComp>()->transform = followShip;


	m_quadContr.Show();
	m_lightContr.Show();
	m_dirlightContr.Show();

	m_pointLight.GetComponent<PointLightComp>()->pointLight.position = m_lightContr.slider1.value;
	m_pointLight.GetComponent<PointLightComp>()->pointLight.color = m_lightContr.slider2.value;

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
