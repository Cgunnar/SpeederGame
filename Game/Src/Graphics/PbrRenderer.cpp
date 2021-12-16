#include "pch.hpp"
#include "PbrRenderer.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"

PbrRenderer::PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes) : m_sharedRenderResources(sharedRes)
{
	m_PS_PBR_AL_MERO_NO_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR_AL_MERO_NO_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_ALB_METROU_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR_ALB_METROU_PointLight.hlsl", ShaderType::PIXELSHADER);

}

void PbrRenderer::Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type)
{
	if ((type & MaterialType::transparent) == MaterialType::transparent)
	{
		assert(false); // fix
	}

	if ((type & MaterialType::PBR_ALBEDO_METROUG_NOR) == MaterialType::PBR_ALBEDO_METROUG_NOR)
	{
		m_PBR_ALBEDO_METROUG_NOR.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PBR_ALBEDO_METROUG) == MaterialType::PBR_ALBEDO_METROUG)
	{
		m_PBR_ALBEDO_METROUG.emplace_back(unitID, worlMatrix, type);
	}


	
}

void PbrRenderer::PreProcess(const VP& viewAndProjMatrix)
{
	m_prePocessed = true;
}

void PbrRenderer::Render(const VP& viewAndProjMatrix)
{
	if (!m_prePocessed) PreProcess(viewAndProjMatrix);



	auto rendRes = m_sharedRenderResources.lock();

	LowLvlGfx::Bind(rendRes->m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::PIXELSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_linearWrapSampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	RenderPBR_ALBEDO_METROUG_NOR();
	RenderPBR_ALBEDO_METROUG();



	m_prePocessed = false;
}

void PbrRenderer::RenderPBR_ALBEDO_METROUG_NOR()
{

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShaderNormalMap);
	LowLvlGfx::Bind(m_PS_PBR_AL_MERO_NO_PointLight);
	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());

	for (auto& unit : m_PBR_ALBEDO_METROUG_NOR)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PBR_ALBEDO_METROUG_NOR) == MaterialType::PBR_ALBEDO_METROUG_NOR);
		const PBR_ALBEDO_METROUG_NOR& matVariant = std::get<PBR_ALBEDO_METROUG_NOR>(rendUnit.material.materialVariant);
		auto albedoTex = assetMan.GetTexture2D(matVariant.albedoTextureID);
		auto normalTex = assetMan.GetTexture2D(matVariant.normalTextureID);
		auto matallicRoughnessText = assetMan.GetTexture2D(matVariant.matallicRoughnessTextureID);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::BindSRV(normalTex, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_ALBEDO_METROUG_NOR.clear();
}

void PbrRenderer::RenderPBR_ALBEDO_METROUG()
{
	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShader);
	LowLvlGfx::Bind(m_PS_PBR_ALB_METROU_PointLight);
	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());

	for (auto& unit : m_PBR_ALBEDO_METROUG)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PBR_ALBEDO_METROUG) == MaterialType::PBR_ALBEDO_METROUG);
		const PBR_ALBEDO_METROUG& matVariant = std::get<PBR_ALBEDO_METROUG>(rendUnit.material.materialVariant);
		auto albedoTex = assetMan.GetTexture2D(matVariant.albedoTextureID);
		auto matallicRoughnessText = assetMan.GetTexture2D(matVariant.matallicRoughnessTextureID);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_ALBEDO_METROUG.clear();
}
