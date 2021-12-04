#include "pch.hpp"
#include "Renderer.h"
#include "rfEntity.hpp"
#include "RenderComponents.h"
#include "StandardComponents.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "RimfrostMath.hpp"

using namespace rfm;

struct alignas(16) PointLight
{
	Vector3 position{ 0, 0, 0 };
	float lightStrength = 1;
	Vector3 color = { 1, 1, 1 };
	float constantAttenuation = 1;
	float LinearAttenuation = 0.1f;
	float exponentialAttenuation = 0.1f;
} pointLight;

struct alignas(16) PhongMaterial
{
	Vector4 ka;
	Vector4 kd;
	Vector3 ks;
	float shininess;
};



Renderer::Renderer()
{
	m_vp.P = Matrix(PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);

	m_worldMatrixCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	m_phongMaterialCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PhongMaterial), BufferDesc::USAGE::DYNAMIC });
	m_pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PointLight), BufferDesc::USAGE::DYNAMIC }, &pointLight);
	m_vpCB = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });


	m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	m_phongPS = LowLvlGfx::CreateShader("Src/Shaders/PS_Phong_DiffTexture_singleLight.hlsl", ShaderType::PIXELSHADER);
	m_sampler = LowLvlGfx::CreateSampler(standardSamplers::g_linear_wrap);
}

Renderer::~Renderer()
{
}


void Renderer::Render(rfe::Entity& camera)
{
	LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());

	m_vp.V = inverse(*camera.getComponent<TransformComp>());
	LowLvlGfx::UpdateBuffer(m_vpCB, &m_vp);

	PhongRender(camera);
}

void Renderer::PhongRender(rfe::Entity& camera)
{
	
	LowLvlGfx::Bind(m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(m_vpCB, ShaderType::PIXELSHADER, 1);

	LowLvlGfx::Bind(m_vertexShader);
	LowLvlGfx::Bind(m_phongPS);
	LowLvlGfx::Bind(m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(m_sampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& m : rfe::EntityReg::getComponentArray<DiffuseTexturMaterialComp>())
	{
		auto diffuseTex = AssetManager::Get().GetTexture2D(m.textureID);
		if (auto ib = rfe::EntityReg::getComponent<IndexedMeshComp>(m.getEntityID()); ib)
		{
			PhongMaterial mat;
			mat.ks = m.specularColor;
			mat.shininess = m.shininess;
			LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
			LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);

			LowLvlGfx::Bind(ib->vertexBuffer);
			LowLvlGfx::Bind(ib->indexBuffer);
			LowLvlGfx::UpdateBuffer(m_worldMatrixCB, &rfe::EntityReg::getComponent<TransformComp>(m.getEntityID())->transform);
			LowLvlGfx::Bind(m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
			LowLvlGfx::BindSRV(AssetManager::Get().GetTexture2D(m.textureID), ShaderType::PIXELSHADER, 0);
			LowLvlGfx::DrawIndexed(ib->indexBuffer.GetIndexCount(), 0, 0);
		}

	}
}
