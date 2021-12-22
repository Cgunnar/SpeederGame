#include "pch.hpp"
#include "PbrRenderer.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "StandardComponents.h"

using namespace rfm;


struct alignas(16) PbrMaterial
{
	rfm::Vector4 albedoFactor;
	rfm::Vector3 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
};


PbrRenderer::PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes) : m_sharedRenderResources(sharedRes)
{
	m_PS_PBR_AL_MERO_NO_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR_AL_MERO_NO_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_ALB_METROU_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR_ALB_METROU_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_NOR_EMIS_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR_NOR_EMIS_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR = LowLvlGfx::CreateShader("Src/Shaders/PS_PBR.hlsl", ShaderType::PIXELSHADER);

	BufferDesc desc;
	desc.size = sizeof(PbrMaterial);
	desc.usage = BufferDesc::USAGE::DYNAMIC;
	m_pbrCB = LowLvlGfx::CreateConstantBuffer(desc);


	//create blendstate
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.IndependentBlendEnable = TRUE;
	auto& b = blendDesc.RenderTarget[0];
	b.BlendEnable = TRUE;
	b.BlendOp = D3D11_BLEND_OP_ADD;
	b.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	b.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	b.DestBlendAlpha = D3D11_BLEND_ZERO;
	b.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	b.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	b.SrcBlendAlpha = D3D11_BLEND_ZERO;
	//fix default for rest
	blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[6].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[7].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_BlendState = LowLvlGfx::Create(blendDesc);

	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;

	m_alphaToCovBlend = LowLvlGfx::Create(blendDesc);

	//create rasterizers
	D3D11_RASTERIZER_DESC rzDesc = {};
	rzDesc.CullMode = D3D11_CULL_NONE;
	rzDesc.FillMode = D3D11_FILL_SOLID;
	m_noBackFaceCullRasterizer = LowLvlGfx::Create(rzDesc);
}

void PbrRenderer::Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type)
{
	if ((type & MaterialType::PBR_ALBEDO_METROUG_NOR) == MaterialType::PBR_ALBEDO_METROUG_NOR)
	{
		m_PBR_ALBEDO_METROUG_NOR.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PBR_ALBEDO_METROUG) == MaterialType::PBR_ALBEDO_METROUG)
	{
		m_PBR_ALBEDO_METROUG.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PBR_ALBEDO_METROUG_NOR_EMIS) == MaterialType::PBR_ALBEDO_METROUG_NOR_EMIS)
	{
		m_PBR_ALBEDO_METROUG_NOR_EMIS.emplace_back(unitID, worlMatrix, type);
	}
	else if ((type & MaterialType::PBR_NO_TEXTURES) == MaterialType::PBR_NO_TEXTURES)
	{
		m_PBR_NO_TEXTURES.emplace_back(unitID, worlMatrix, type);
	}
}

void PbrRenderer::PreProcess(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag)
{
	m_prePocessed = true;

}

void PbrRenderer::Render(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag)
{
	if (!m_prePocessed) PreProcess(viewAndProjMatrix, camera, flag);

	

	auto rendRes = m_sharedRenderResources.lock();

	LowLvlGfx::Bind(rendRes->m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::PIXELSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_linearWrapSampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	HandleRenderFlag(flag);

	RenderPBR_ALBEDO_METROUG_NOR_EMIS(flag);
	RenderPBR_ALBEDO_METROUG_NOR(flag);
	RenderPBR_ALBEDO_METROUG(flag);
	RenderPBR_NO_TEXTURES(flag);



	m_prePocessed = false;
}

void PbrRenderer::RenderPBR_ALBEDO_METROUG_NOR(RenderFlag flag)
{
	if (m_PBR_ALBEDO_METROUG_NOR.empty()) return;

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

		PbrMaterial cMat;
		cMat.albedoFactor = matVariant.rgba;
		cMat.roughnessFactor = matVariant.roughness;
		cMat.metallicFactor = matVariant.metallic;
		cMat.emissiveFactor = matVariant.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_ALBEDO_METROUG_NOR.clear();
}

void PbrRenderer::RenderPBR_ALBEDO_METROUG(RenderFlag flag)
{
	if (m_PBR_ALBEDO_METROUG.empty()) return;

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

		PbrMaterial cMat;
		cMat.albedoFactor = matVariant.rgba;
		cMat.roughnessFactor = matVariant.roughness;
		cMat.metallicFactor = matVariant.metallic;
		cMat.emissiveFactor = matVariant.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_ALBEDO_METROUG.clear();
}

void PbrRenderer::RenderPBR_ALBEDO_METROUG_NOR_EMIS(RenderFlag flag)
{
	if (m_PBR_ALBEDO_METROUG_NOR_EMIS.empty()) return;

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShaderNormalMap);
	LowLvlGfx::Bind(m_PS_PBR_NOR_EMIS_PointLight);

	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());

	for (auto& unit : m_PBR_ALBEDO_METROUG_NOR_EMIS)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PBR_ALBEDO_METROUG_NOR_EMIS) == MaterialType::PBR_ALBEDO_METROUG_NOR_EMIS);
		const PBR_ALBEDO_METROUG_NOR_EMIS& matVariant = std::get<PBR_ALBEDO_METROUG_NOR_EMIS>(rendUnit.material.materialVariant);
		auto albedoTex = assetMan.GetTexture2D(matVariant.albedoTextureID);
		auto normalTex = assetMan.GetTexture2D(matVariant.normalTextureID);
		auto matallicRoughnessText = assetMan.GetTexture2D(matVariant.matallicRoughnessTextureID);
		auto emissiveText = assetMan.GetTexture2D(matVariant.emissiveTextureID);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::BindSRV(normalTex, ShaderType::PIXELSHADER, 2);
		LowLvlGfx::BindSRV(emissiveText, ShaderType::PIXELSHADER, 3);

		PbrMaterial cMat;
		cMat.albedoFactor = matVariant.rgba;
		cMat.roughnessFactor = matVariant.roughness;
		cMat.metallicFactor = matVariant.metallic;
		cMat.emissiveFactor = matVariant.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_ALBEDO_METROUG_NOR_EMIS.clear();
}

void PbrRenderer::RenderPBR_NO_TEXTURES(RenderFlag flag)
{
	if (m_PBR_NO_TEXTURES.empty()) return;

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShader);
	LowLvlGfx::Bind(m_PS_PBR);
	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());


	for (auto& unit : m_PBR_NO_TEXTURES)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		assert((rendUnit.material.type & MaterialType::PBR_NO_TEXTURES) == MaterialType::PBR_NO_TEXTURES);
		const PBR_NO_TEXTURES& matVariant = std::get<PBR_NO_TEXTURES>(rendUnit.material.materialVariant);

		PbrMaterial cMat;
		cMat.albedoFactor = matVariant.rgba;
		cMat.roughnessFactor = matVariant.roughness;
		cMat.metallicFactor = matVariant.metallic;
		cMat.emissiveFactor = matVariant.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &unit.worldMatrix);
		LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendUnit.subMesh.vb);
		LowLvlGfx::Bind(rendUnit.subMesh.ib);
		LowLvlGfx::DrawIndexed(rendUnit.subMesh.indexCount, rendUnit.subMesh.startIndexLocation, rendUnit.subMesh.baseVertexLocation);
	}

	m_PBR_NO_TEXTURES.clear();
}

void PbrRenderer::HandleRenderFlag(RenderFlag flag)
{
	if (RenderFlag::none == flag)
	{
		LowLvlGfx::UnBindBlendState();
		LowLvlGfx::UnBindRasterizer();
	}
	if ((flag & RenderFlag::alphaToCov) != 0)
	{
		LowLvlGfx::Bind(m_alphaToCovBlend);
	}
	else if ((flag & RenderFlag::alphaBlend) != 0)
	{
		LowLvlGfx::Bind(m_BlendState);
	}

	if ((flag & RenderFlag::noBackFaceCull) != 0)
	{
		LowLvlGfx::Bind(m_noBackFaceCullRasterizer);
	}
}
