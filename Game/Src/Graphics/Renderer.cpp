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
	m_worldMatrixCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	m_phongMaterialCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PhongMaterial), BufferDesc::USAGE::DYNAMIC });
	m_pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PointLight), BufferDesc::USAGE::DYNAMIC }, &pointLight);
}

Renderer::~Renderer()
{
}


void Renderer::Render()
{
	PhongRender();
}

void Renderer::PhongRender()
{
	LowLvlGfx::Bind(m_pointLightCB, ShaderType::PIXELSHADER, 0);
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
