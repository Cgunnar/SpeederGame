#include "pch.hpp"
#include "Scene.h"
#include "AssetManager.h"
#include "Geometry.h"
#include "LowLvlGfx.h"
#include "ReadImg.hpp"
#include "AssimpLoader.h"

using namespace rfm;
using namespace rfe;


void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride)
{
	assert(dataPtr);

	subResMipArray = new D3D11_SUBRESOURCE_DATA[mipNumber];
	int SysMemPitch = stride;

	for (int i = 0; i < mipNumber; i++)
	{
		subResMipArray[i].pSysMem = dataPtr;
		subResMipArray[i].SysMemPitch = SysMemPitch;
		subResMipArray[i].SysMemSlicePitch = 0;
		SysMemPitch >>= 1;
	}
}

Scene::Scene()
{
	AssimpLoader a;
	auto m = a.loadStaticModel("Assets/Models/Sun/Sun.obj");


	m_camera = EntityReg::createEntity();
	m_camera.addComponent(TransformComp());
	m_camera.getComponent<TransformComp>()->transform.setTranslation(0, 0, 1);
	m_camera.getComponent<TransformComp>()->transform.setRotationDeg(0, 0, 0);

	m_pointLight = EntityReg::createEntity();
	m_pointLight.addComponent(TransformComp());
	m_pointLight.addComponent(PointLightComp());

	Geometry::Quad_POS_NOR_UV quad2;
	m_quad = EntityReg::createEntity();
	m_quad.addComponent(TransformComp());
	RenderComp* rendComp = m_quad.addComponent(RenderComp());
	rendComp->renderPass = RenderComp::RenderPassEnum::phong;
	rendComp->subMeshID = static_cast<MeshID>(SimpleMesh::Quad);


	MyImageStruct im;
	readImage(im, "Assets/Hej.png");
	D3D11_TEXTURE2D_DESC desc;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Height = im.height;
	desc.Width = im.width;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = im.mipNumber;

	D3D11_SUBRESOURCE_DATA* subResMipArray = nullptr;
	SetSubResDataMips(im.imagePtr, subResMipArray, im.mipNumber, im.stride);
	std::shared_ptr<Texture2D> myTexture = LowLvlGfx::CreateTexture2D(desc, subResMipArray);
	delete[] subResMipArray;


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	LowLvlGfx::CreateSRV(myTexture, &srvDesc);

	m_quad.addComponent(DiffuseTexturMaterialComp())->textureID = AssetManager::Get().AddTexture2D(myTexture);
	m_quad.getComponent<DiffuseTexturMaterialComp>()->specularColor = { 1,0,0 };

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
