#include "pch.hpp"
#include "PhongRenderer.h"
#include "AssetManager.h"

struct alignas(16) PhongMaterial
{
	rfm::Vector4 ka;
	rfm::Vector4 kd;
	rfm::Vector3 ks;
	float shininess;
};

PhongRenderer::PhongRenderer(std::weak_ptr<SharedRenderResources> sharedRes) : m_sharedRenderResources(sharedRes)
{
	m_PS_Phong_DiffTexture_singleLight = LowLvlGfx::CreateShader("Src/Shaders/PS_Phong_DiffTexture_singleLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_Phong_singlePointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_Phong_singlePointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_Phong_DiffTex_NorTex_singleLight = LowLvlGfx::CreateShader("Src/Shaders/PS_Phong_DiffTex_NorTex_singleLight.hlsl", ShaderType::PIXELSHADER);
	m_phongMaterialCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PhongMaterial), BufferDesc::USAGE::DYNAMIC });
}

void PhongRenderer::Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type)
{
	if ((type & MaterialType::transparent) == MaterialType::transparent)
	{
		assert(false); // fix
	}


	if ((type & MaterialType::PhongMaterial_Color) == MaterialType::PhongMaterial_Color)
	{
		m_colorUnits.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PhongMaterial_DiffTex) == MaterialType::PhongMaterial_DiffTex)
	{
		m_diffTextureUnits.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PhongMaterial_DiffTex_NormTex) == MaterialType::PhongMaterial_DiffTex_NormTex)
	{
		m_PhongMaterial_DiffTex_NormTex.emplace_back(unitID, worlMatrix, type);
	}
}

void PhongRenderer::PreProcess(const VP& viewAndProjMatrix)
{
	m_prePocessed = true;


}

void PhongRenderer::Render(const VP& viewAndProjMatrix)
{
	if (!m_prePocessed) PreProcess(viewAndProjMatrix);

	auto rendRes = m_sharedRenderResources.lock();
	LowLvlGfx::Bind(rendRes->m_vertexShader);
	LowLvlGfx::Bind(rendRes->m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::PIXELSHADER, 1);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	LowLvlGfx::Bind(rendRes->m_linearWrapSampler, ShaderType::PIXELSHADER, 0);

	RenderWithColorOnly();
	RenderWithDiffuseTexture();
	RenderPhongMaterial_DiffTex_NormTex();

	m_prePocessed = false;
}

void PhongRenderer::RenderWithColorOnly()
{
	auto rendRes = m_sharedRenderResources.lock();
	AssetManager& assetMan = AssetManager::Get();
	PhongMaterial mat;

	LowLvlGfx::Bind(m_PS_Phong_singlePointLight);

	for (auto& unit : m_colorUnits)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PhongMaterial_Color) == MaterialType::PhongMaterial_Color);
		const PhongMaterial_Color& matVariant = std::get<PhongMaterial_Color>(rendUnit.material.materialVariant);
		mat.ka = rfm::Vector4(matVariant.ambientColor, 1);
		mat.kd = rfm::Vector4(matVariant.diffuseColor, 1);
		mat.ks = matVariant.specularColor;
		mat.shininess = matVariant.shininess;

		LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}
	m_colorUnits.clear();
}

void PhongRenderer::RenderWithDiffuseTexture()
{
	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	PhongMaterial mat;

	LowLvlGfx::Bind(m_PS_Phong_DiffTexture_singleLight);

	for (auto& unit : m_diffTextureUnits)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PhongMaterial_DiffTex) == MaterialType::PhongMaterial_DiffTex);
		const PhongMaterial_DiffTex& matVariant = std::get<PhongMaterial_DiffTex>(rendUnit.material.materialVariant);
		mat.ks = matVariant.specularColor;
		mat.shininess = matVariant.shininess;
		auto diffTex = assetMan.GetTexture2D(matVariant.diffuseTextureID);

		LowLvlGfx::BindSRV(diffTex, ShaderType::PIXELSHADER, 0);

		LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}
	m_diffTextureUnits.clear();
}

void PhongRenderer::RenderPhongMaterial_DiffTex_NormTex()
{
	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	PhongMaterial mat;

	LowLvlGfx::Bind(rendRes->m_vertexShaderNormalMap);
	LowLvlGfx::Bind(m_PS_Phong_DiffTex_NorTex_singleLight);

	for (auto& unit : m_PhongMaterial_DiffTex_NormTex)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PhongMaterial_DiffTex_NormTex) == MaterialType::PhongMaterial_DiffTex_NormTex);
		const PhongMaterial_DiffTex_NormTex& matVariant = std::get<PhongMaterial_DiffTex_NormTex>(rendUnit.material.materialVariant);
		mat.ks = matVariant.specularColor;
		mat.shininess = matVariant.shininess;
		auto diffTex = assetMan.GetTexture2D(matVariant.diffuseTextureID);
		auto normTex = assetMan.GetTexture2D(matVariant.normalTextureID);

		LowLvlGfx::BindSRV(diffTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(normTex, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}
	m_PhongMaterial_DiffTex_NormTex.clear();


}
