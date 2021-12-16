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

using namespace rfm;
using namespace rfe;


Scene::Scene()
{
	m_pistol = EntityReg::createEntity();
	m_pistol.addComponent(TransformComp())->transform.setTranslation(2, 1, 5);
	m_pistol.getComponent<TransformComp>()->transform.setScale(0.01);
	m_pistol.getComponent<TransformComp>()->transform.setRotationDeg(90, 90, 0);
	m_pistol.addComponent(RenderModelComp("Assets/Models/cerberus/scene.gltf", RenderPassEnum::pbr));
	//m_pistol.addComponent(RenderModelComp("Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf", RenderPassEnum::pbr));
	//m_pistol.addComponent(RenderModelComp("Assets/Models/material_ball/scene.gltf", RenderPassEnum::pbr));


	m_nanosuit = EntityReg::createEntity();
	m_nanosuit.addComponent(TransformComp())->transform.setTranslation(0, 0, 5);
	m_nanosuit.getComponent<TransformComp>()->transform.setScale(0.1);
	m_nanosuit.addComponent(RenderModelComp("Assets/Models/nanosuit/nanosuit.obj", RenderPassEnum::phong));

	m_arrow = EntityReg::createEntity();
	m_arrow.addComponent(TransformComp());
	m_arrow.addComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj", RenderPassEnum::phong));

	m_brickWallFloor = EntityReg::createEntity();
	m_brickWallFloor.addComponent(TransformComp())->transform.setRotationDeg(90, 0, 0);
	m_brickWallFloor.getComponent<TransformComp>()->transform.setScale(10);
	m_brickWallFloor.addComponent(RenderModelComp("Assets/Models/brick_wall/brick_wall.obj", RenderPassEnum::phong));

	m_camera = EntityReg::createEntity();
	m_camera.addComponent(TransformComp());
	m_camera.getComponent<TransformComp>()->transform.setTranslation(0, 1, -5);
	m_camera.getComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);

	m_camera.addScript(CameraControllerScript());

	m_pointLight = EntityReg::createEntity();
	m_pointLight.addComponent(TransformComp());
	m_pointLight.addComponent(PointLightComp());
	m_pointLight.getComponent<PointLightComp>()->pointLight.lightStrength = 10;

	
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
	SubMesh quadMeshCopy = AssetManager::Get().GetMesh(SimpleMesh::Quad);
	
	rendComp->renderUnitID = AssetManager::Get().AddRenderUnit(quadMeshCopy, quadMat);
	


	m_quadContr.slider1.ChangeDefaultValues({ 0,0,8 });

	m_lightContr.slider1.ChangeDefaultValues({ 0,1,4 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
	EntityReg::RunScripts<CameraControllerScript>(dt);

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
