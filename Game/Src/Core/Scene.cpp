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
	m_camera = EntityReg::createEntity();
	m_camera.addComponent<TransformComp>()->transform.setTranslation(0, 2, -4);
	m_camera.addComponent<CameraControllerScript>();

	AssetManager& am = AssetManager::Get();

	TerrainLoader tl;
	tl.CreateTerrainFromBMP("Assets/Textures/noiseTexture.bmp");
	SubMesh terrainMesh(tl.GetVerticesTBN(), tl.GetIndices());

	m_terrain = EntityReg::createEntity();
	m_terrain.addComponent(TransformComp())->transform.setTranslation({ -64, -9, -64 });
	Material terrainMat;
	PBR_ALBEDO_METROUG_NOR desert_rocks;
	//desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/sand/basecolor.jpg", LoadTexFlag::GenerateMips);
	//desert_rocks.matallicRoughnessTextureID = am.LoadTex2D("Assets/Textures/sand/metallic_roughness.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	//desert_rocks.normalTextureID = am.LoadTex2D("Assets/Textures/sand/normal.jpg", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);//desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/sand/basecolor.jpg", LoadTexFlag::GenerateMips);
	desert_rocks.albedoTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/basecolor.png", LoadTexFlag::GenerateMips);
	desert_rocks.matallicRoughnessTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/metallic_roughness.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	desert_rocks.normalTextureID = am.LoadTex2D("Assets/Textures/rock_slab_wall/normal_ogl.png", LoadTexFlag::LinearColorSpace | LoadTexFlag::GenerateMips);
	terrainMat.materialVariant = desert_rocks;
	terrainMat.type = MaterialType::PBR_ALBEDO_METROUG_NOR;
	RenderModelComp renderComp;
	renderComp.SetRenderUnit(am.AddRenderUnit(terrainMesh, terrainMat));
	renderComp.renderPass = RenderPassEnum::pbr;
	m_terrain.addComponent(renderComp);


	this->CreateEntityModel("Assets/Models/MetalRoughSpheres/glTF/pbrSpheres.gltf", { -2, 3, 4 }, { 0, 0, 0 }, 0.2f);

	sky.Init("Assets/Textures/MonValley_Lookout/MonValley_A_LookoutPoint_2k.hdr");

	this->CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 4, 2, 2 }, 0, 0.03f);
	this->CreateEntityModel("Assets/Models/cerberus/scene.gltf", { 3, 2, 4 }, { 0,-70, 0 }, 0.03f);

	m_ship = this->CreateEntityModel("Assets/Models/pbr/ajf-12_dvergr/scene.gltf", { 0, 2, 3 });
	m_ship.addComponent(ShipContollerScript());
	

	CreateEntityModel("Assets/Models/nanosuit/nanosuit.obj", { 2, 1, 5 }, 0, 0.1f);

	m_arrow = EntityReg::createEntity();
	m_arrow.addComponent(TransformComp());
	m_arrow.addComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj", RenderPassEnum::phong));

	m_brickWallFloor = EntityReg::createEntity();
	m_brickWallFloor.addComponent(TransformComp())->transform.setRotationDeg(90, 0, 0);
	m_brickWallFloor.getComponent<TransformComp>()->transform.setScale(10);
	m_brickWallFloor.addComponent<RenderModelComp>("Assets/Models/brick_wall/brick_wall.obj", RenderPassEnum::phong);

	

	m_pointLight = EntityReg::createEntity();
	m_pointLight.addComponent(TransformComp());
	m_pointLight.addComponent(PointLightComp());
	m_pointLight.getComponent<PointLightComp>()->pointLight.lightStrength = 5;

	
	Material quadMat;
	quadMat.type = MaterialType::PhongMaterial_DiffTex;
	PhongMaterial_DiffTex mat;
	mat.specularColor = { 1,0,0 };
	mat.diffuseTextureID = AssetManager::Get().LoadTex2D("Assets/Hej.png", LoadTexFlag::GenerateMips);
	quadMat.materialVariant = mat;

	m_quad = EntityReg::createEntity();
	m_quad.addComponent(TransformComp());
	RenderModelComp* rendComp = m_quad.addComponent(RenderModelComp());
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

	m_ironSphere = EntityReg::createEntity();
	m_ironSphere.addComponent(TransformComp())->transform.setTranslation(0, 2, 0);
	rendComp = m_ironSphere.addComponent(RenderModelComp());
	rendComp->renderPass = RenderPassEnum::pbr;
	SubMesh quadMeshCopy2 = AssetManager::Get().GetMesh(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN);

	rendComp->renderUnitID = AssetManager::Get().AddRenderUnit(quadMeshCopy2, rusteIronMat);
	

	

	m_debugSphere = EntityReg::createEntity();
	m_debugSphere.addComponent<TransformComp>()->transform.setTranslation(-3, 2, 0);
	m_debugSphere.getComponent<TransformComp>()->transform.setRotationDeg(0, -90, 0);

	RenderModelComp rc;
	GID mID = am.LoadMesh("Assets/Models/UV_Sphere/sphere.obj", MeshFormat::POS_NOR_UV_TAN_BITAN);
	SubMesh blendUVSphere = am.GetMesh(mID);
	rc.SetRenderUnit(am.AddRenderUnit(blendUVSphere, rusteIronMat));
	rc.renderPass = RenderPassEnum::pbr;
	m_debugSphere.addComponent(rc);


	m_quadContr.slider1.ChangeDefaultValues({ 0,0,8 });

	m_lightContr.slider1.ChangeDefaultValues({ 0,2,-9 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{

	



	EntityReg::RunScripts<CameraControllerScript, ShipContollerScript>(dt);

	Transform followShip = m_ship.getComponent<TransformComp>()->transform;
	followShip.translateL(0, 1, -4);
	//m_camera.getComponent<TransformComp>()->transform = followShip;


	m_quadContr.Show();
	m_lightContr.Show();

	m_quad.getComponent<TransformComp>()->transform.setTranslation(m_quadContr.slider1.value);
	m_quad.getComponent<TransformComp>()->transform.setRotation(m_quadContr.slider2.value.x, m_quadContr.slider2.value.y, m_quadContr.slider2.value.z);

	m_pointLight.getComponent<PointLightComp>()->pointLight.position = m_lightContr.slider1.value;
	m_pointLight.getComponent<PointLightComp>()->pointLight.color = m_lightContr.slider2.value;

}

rfe::Entity& Scene::GetCamera()
{
	return m_camera;
}

rfe::Entity Scene::CreateEntityModel(const std::string path, Vector3 pos, Vector3 rotDeg, Vector3 scale)
{
	m_entities.push_back(EntityReg::createEntity());
	Transform& t = m_entities.back().addComponent<TransformComp>()->transform;
	t.setTranslation(pos);
	t.setRotationDeg(rotDeg.x, rotDeg.y, rotDeg.z);
	t.setScale(scale.x, scale.y, scale.z);

	if(std::filesystem::path(path).extension() == ".gltf")
		m_entities.back().addComponent<RenderModelComp>(path, RenderPassEnum::pbr);
	else
		m_entities.back().addComponent<RenderModelComp>(path, RenderPassEnum::phong);

	return m_entities.back();
}
