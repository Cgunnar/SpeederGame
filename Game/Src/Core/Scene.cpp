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
#include "TerrainLoader.h"

using namespace rfm;
using namespace rfe;


Scene::Scene()
{
	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 2, -4);
	m_camera.AddComponent<CameraControllerScript>();

	AssetManager& am = AssetManager::Get();

	TerrainLoader tl;
	tl.CreateTerrainFromBMP("Assets/Textures/noiseTexture.bmp");
	SubMesh terrainMesh(tl.GetVerticesTBN(), tl.GetIndices());

	m_terrain = EntityReg::CreateEntity();
	m_terrain.AddComponent(TransformComp())->transform.setTranslation({ -64, -9, -64 });
	Material terrainMat;
	PBR_ALBEDO_METROUG_NOR desert_rocks;
	desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/sand/basecolor.jpg", LoadTexFlag::GenerateMips);
	desert_rocks.matallicRoughnessTextureID = am.LoadTex2D("Assets/Textures/sand/metallic_roughness.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	desert_rocks.normalTextureID = am.LoadTex2D("Assets/Textures/sand/normal.jpg", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);//desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/sand/basecolor.jpg", LoadTexFlag::GenerateMips);
	//desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/basecolor.png", LoadTexFlag::GenerateMips);
	//desert_rocks.matallicRoughnessTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/metallic_roughness.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	//desert_rocks.normalTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/normal_ogl.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	terrainMat.materialVariant = desert_rocks;
	terrainMat.type = MaterialType::PBR_ALBEDO_METROUG_NOR;
	RenderModelComp renderComp;
	renderComp.SetRenderUnit(am.AddRenderUnit(terrainMesh, terrainMat));
	renderComp.renderPass = RenderPassEnum::pbr;
	m_terrain.AddComponent(renderComp);


	sky.Init("Assets/Textures/MonValley_Lookout/MonValley_A_LookoutPoint_2k.hdr");


	CreateEntityModel("Assets/Models/MetalRoughSpheres/glTF/pbrSpheres.gltf", { -2, 3, 4 }, { 0, 0, 0 }, 0.2f);
	CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 4, 2, 2 }, 0, 0.03f);
	CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 3, 2, 4 }, { 0,-70, 0 }, 0.03f);
	CreateEntityModel("Assets/Models/nanosuit/nanosuit.obj", { 2, 1, 5 }, 0, 0.1f);
	CreateEntityModel("Assets/Models/brick_wall/brick_wall.obj", 0, { 90, 0, 0}, 10);

	m_ship = CreateEntityModel("Assets/Models/pbr/ajf-12_dvergr/scene.gltf", { 0, 2, 3 });
	m_ship.AddComponent<ShipContollerScript>();
	


	m_arrow = EntityReg::CreateEntity();
	m_arrow.AddComponent(TransformComp());
	m_arrow.AddComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj", RenderPassEnum::phong));

	

	m_pointLight = EntityReg::CreateEntity();
	m_pointLight.AddComponent<TransformComp>();
	m_pointLight.AddComponent<PointLightComp>()->pointLight.lightStrength = 5;

	m_dirLight = EntityReg::CreateEntity();
	m_dirLight.AddComponent<TransformComp>();
	m_dirLight.AddComponent<DirectionalLightComp>()->dirLight.color = { 1, 0.87f, 0.23f };

	
	Material quadMat;
	quadMat.type = MaterialType::PhongMaterial_DiffTex;
	PhongMaterial_DiffTex mat;
	mat.specularColor = { 1,0,0 };
	mat.diffuseTextureID = AssetManager::Get().LoadTex2D("Assets/Hej.png", LoadTexFlag::GenerateMips);
	quadMat.materialVariant = mat;

	m_quad = EntityReg::CreateEntity();
	m_quad.AddComponent(TransformComp());
	RenderModelComp* rendComp = m_quad.AddComponent(RenderModelComp());
	rendComp->renderPass = RenderPassEnum::phong;
	SubMesh quadMeshCopy = AssetManager::Get().GetMesh(SimpleMesh::Quad_POS_NOR_UV);
	rendComp->renderUnitID = AssetManager::Get().AddRenderUnit(quadMeshCopy, quadMat);



	Material rusteIronMat;
	rusteIronMat.type = MaterialType::PBR_ALBEDO_METROUG_NOR;
	PBR_ALBEDO_METROUG_NOR pbrMat;
	pbrMat.matallicRoughnessTextureID = AssetManager::Get().LoadTex2D("Assets/Textures/rustediron/metallic_roughness.png", LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
	pbrMat.normalTextureID = AssetManager::Get().LoadTex2D("Assets/Textures/rustediron/normal.png", LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
	pbrMat.albedoTextureID = AssetManager::Get().LoadTex2D("Assets/Textures/rustediron/basecolor.png", LoadTexFlag::GenerateMips);
	rusteIronMat.materialVariant = pbrMat;

	m_ironSphere = EntityReg::CreateEntity();
	m_ironSphere.AddComponent(TransformComp())->transform.setTranslation(0, 2, 0);
	rendComp = m_ironSphere.AddComponent(RenderModelComp());
	rendComp->renderPass = RenderPassEnum::pbr;
	SubMesh quadMeshCopy2 = AssetManager::Get().GetMesh(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN);

	rendComp->renderUnitID = AssetManager::Get().AddRenderUnit(quadMeshCopy2, rusteIronMat);
	

	

	m_debugSphere = EntityReg::CreateEntity();
	m_debugSphere.AddComponent<TransformComp>()->transform.setTranslation(-3, 2, 0);
	m_debugSphere.GetComponent<TransformComp>()->transform.setRotationDeg(0, -90, 0);

	RenderModelComp rc;
	GID mID = am.LoadMesh("Assets/Models/UV_Sphere/sphere.obj", MeshFormat::POS_NOR_UV_TAN_BITAN);
	SubMesh blendUVSphere = am.GetMesh(mID);
	rc.SetRenderUnit(am.AddRenderUnit(blendUVSphere, rusteIronMat));
	rc.renderPass = RenderPassEnum::pbr;
	m_debugSphere.AddComponent(rc);


	m_quadContr.slider1.ChangeDefaultValues({ 0,0,8 });

	m_lightContr.slider1.ChangeDefaultValues({ 0,2,-9 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);

	m_dirlightContr.slider1.ChangeDefaultValues({ 0.3f ,-1, 0.5f}, -1, 1);
	m_dirlightContr.slider2.ChangeDefaultValues(m_dirLight.GetComponent<DirectionalLightComp>()->dirLight.color, 0, 1);

}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{

	



	EntityReg::RunScripts<CameraControllerScript, ShipContollerScript>(dt);

	Transform followShip = m_ship.GetComponent<TransformComp>()->transform;
	followShip.translateL(0, 1, -4);
	//m_camera.getComponent<TransformComp>()->transform = followShip;


	m_quadContr.Show();
	m_lightContr.Show();
	m_dirlightContr.Show();

	m_quad.GetComponent<TransformComp>()->transform.setTranslation(m_quadContr.slider1.value);
	m_quad.GetComponent<TransformComp>()->transform.setRotation(m_quadContr.slider2.value.x, m_quadContr.slider2.value.y, m_quadContr.slider2.value.z);

	m_pointLight.GetComponent<PointLightComp>()->pointLight.position = m_lightContr.slider1.value;
	m_pointLight.GetComponent<PointLightComp>()->pointLight.color = m_lightContr.slider2.value;

	m_dirLight.GetComponent<DirectionalLightComp>()->dirLight.dir = m_dirlightContr.slider1.value;
	m_dirLight.GetComponent<DirectionalLightComp>()->dirLight.color = m_dirlightContr.slider2.value;

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

	if(std::filesystem::path(path).extension() == ".gltf")
		m_entities.back().AddComponent<RenderModelComp>(path, RenderPassEnum::pbr);
	else
		m_entities.back().AddComponent<RenderModelComp>(path, RenderPassEnum::phong);

	return m_entities.back();
}
