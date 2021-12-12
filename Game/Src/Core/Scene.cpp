#include "pch.hpp"
#include "Scene.h"
#include "AssetManager.h"
#include "Geometry.h"
#include "LowLvlGfx.h"
#include "ReadImg.hpp"
#include "AssimpLoader.h"
#include "GraphicsHelperFunctions.h"
#include "GraphicsResources.h"

using namespace rfm;
using namespace rfe;


struct MyScript : rfe::NativeScriptComponent<MyScript>
{
	std::string name = "hej";
	void OnUpdate(float dt)
	{
		std::cout << name << name << std::endl;
	}
};

struct MyScript2 : rfe::NativeScriptComponent<MyScript2>
{
	std::string name = "okok";
	void OnUpdate(float dt)
	{
		std::cout << name << name << std::endl;
	}
	void OnUpdateLate(float dt)
	{
		std::cout << "2312" << std::endl;
	}
};


Scene::Scene()
{
	m_nanosuit = EntityReg::createEntity();
	m_nanosuit.addComponent(TransformComp())->transform.setTranslation(0, 0, 8);
	m_nanosuit.getComponent<TransformComp>()->transform.setScale(0.05);
	m_nanosuit.addComponent(RenderModelComp("Assets/Models/nanosuit/nanosuit.obj"));


	m_arrow = EntityReg::createEntity();
	m_arrow.addComponent(TransformComp());
	m_arrow.addComponent(RenderModelComp("Assets/Models/Arrows/DXRefSys.obj"));
	m_arrow.addScript(MyScript2());


	m_camera = EntityReg::createEntity();
	m_camera.addComponent(TransformComp());
	m_camera.getComponent<TransformComp>()->transform.setTranslation(0, 0, -5);
	m_camera.getComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);
	//m_camera.addComponent(NativeScriptComponent<MyScript>());
	m_camera.addScript(MyScript());

	m_pointLight = EntityReg::createEntity();
	m_pointLight.addComponent(TransformComp());
	m_pointLight.addComponent(PointLightComp());

	
	Material quadMat;
	quadMat.type = MaterialType::PhongMaterial_DiffTex;
	PhongMaterial_DiffTex mat;
	mat.specularColor = { 1,0,0 };
	mat.diffuseTextureID = AssetManager::Get().LoadTex2D("Assets/Hej.png", true, true);
	quadMat.materialVariant = mat;

	m_quad = EntityReg::createEntity();
	m_quad.addComponent(TransformComp());
	RenderModelComp* rendComp = m_quad.addComponent(RenderModelComp());
	rendComp->renderPass = RenderModelComp::RenderPassEnum::phong;
	SubMesh quadMeshCopy = AssetManager::Get().GetMesh(SimpleMesh::Quad);
	
	rendComp->renderUnitID = AssetManager::Get().AddRenderUnit(quadMeshCopy, quadMat);
	


	m_quadContr.slider1.ChangeDefaultValues({ 0,0,8 });

	m_lightContr.slider1.ChangeDefaultValues({ 0,0,5 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
	EntityReg::RunScripts<MyScript, MyScript2>(dt);

	m_quadContr.Show();
	m_lightContr.Show();
	m_cameraContr.Show();

	m_quad.getComponent<TransformComp>()->transform.setTranslation(m_quadContr.slider1.value);
	m_quad.getComponent<TransformComp>()->transform.setRotation(m_quadContr.slider2.value.x, m_quadContr.slider2.value.y, m_quadContr.slider2.value.z);

	m_camera.getComponent<TransformComp>()->transform.setTranslation(m_cameraContr.slider1.value);
	m_camera.getComponent<TransformComp>()->transform.setRotation(m_cameraContr.slider2.value.x, m_cameraContr.slider2.value.y, m_cameraContr.slider2.value.z);

	m_pointLight.getComponent<PointLightComp>()->pointLight.position = m_lightContr.slider1.value;
	m_pointLight.getComponent<PointLightComp>()->pointLight.color = m_lightContr.slider2.value;

}

rfe::Entity& Scene::GetCamera()
{
	return m_camera;
}
