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


Scene::Scene()
{
	auto sunModelID = AssetManager::Get().LoadModel("Assets/Models/Sun/Sun.obj");
	Model& sunModel = AssetManager::Get().GetModel(sunModelID);

	m_sun = EntityReg::createEntity();
	m_sun.addComponent(TransformComp())->transform.setTranslation(0, 0, 8);
	RenderComp* rendCompSun = m_sun.addComponent(RenderComp());
	rendCompSun->renderPass = RenderComp::RenderPassEnum::phong;
	rendCompSun->renderUnit.subMesh = sunModel.subModels[0].renderUnits[0].subMesh;
	rendCompSun->renderUnit.material = sunModel.subModels[0].renderUnits[0].material;
	rendCompSun->ModelID = sunModelID;


	m_camera = EntityReg::createEntity();
	m_camera.addComponent(TransformComp());
	m_camera.getComponent<TransformComp>()->transform.setTranslation(0, 0, 1);
	m_camera.getComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);

	m_pointLight = EntityReg::createEntity();
	m_pointLight.addComponent(TransformComp());
	m_pointLight.addComponent(PointLightComp());

	Geometry::Quad_POS_NOR_UV quad2;
	MyImageStruct im;
	readImage(im, "Assets/Hej.png");

	
	Material quadMat;
	quadMat.diffuseTextureID = AssetManager::Get().LoadTex2D("Assets/Hej.png", true, true);
	quadMat.specularColor = { 1,0,0 };

	m_quad = EntityReg::createEntity();
	m_quad.addComponent(TransformComp());
	RenderComp* rendComp = m_quad.addComponent(RenderComp());
	rendComp->renderPass = RenderComp::RenderPassEnum::phong;
	rendComp->renderUnit.subMesh = static_cast<SubMeshID>(SimpleMesh::Quad);
	rendComp->renderUnit.material = quadMat;
	


	m_quadContr.slider1.ChangeDefaultValues({ 0,0,8 });

	m_lightContr.slider1.ChangeDefaultValues({ 0,0,5 });
	m_lightContr.slider2.ChangeDefaultValues({ 1,1,1 }, 0, 1);
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
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
