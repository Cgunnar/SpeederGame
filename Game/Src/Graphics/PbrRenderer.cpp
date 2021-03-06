#include "pch.hpp"
#include "PbrRenderer.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "StandardComponents.h"

using namespace rfm;


struct alignas(16) PbrMaterialCBStruct
{
	rfm::Vector4 albedoFactor;
	rfm::Vector3 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
};


PbrRenderer::PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes) : m_sharedRenderResources(sharedRes)
{
	m_PS_PBR_AL_MERO_NO_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_AL_MERO_NO_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_ALB_METROU_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_ALB_METROU_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_NOR_EMIS_PointLight = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_NOR_EMIS_PointLight.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_NOTEXTURES = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_NOTEXTURES.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_AL = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_AL.hlsl", ShaderType::PIXELSHADER);
	m_PS_PBR_AL_NOR = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_PBR_AL_NOR.hlsl", ShaderType::PIXELSHADER);
	m_PS_terrainShading = LowLvlGfx::CreateShader("Src/Shaders/PBR/PS_terrainShading.hlsl", ShaderType::PIXELSHADER);

	BufferDesc desc;
	desc.size = sizeof(PbrMaterialCBStruct);
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
	rzDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_noBackFaceCullAndWireframeRasterizer = LowLvlGfx::Create(rzDesc);
	rzDesc.CullMode = D3D11_CULL_BACK;
	m_wireframeRasterizer = LowLvlGfx::Create(rzDesc);

	m_linearWrapSampler = LowLvlGfx::Create(standardDescriptors::g_sample_linear_wrap);
	m_linearClampSamplerDefault = LowLvlGfx::Create(standardDescriptors::g_sample_linear_clamp);
	m_anisotropicWrapSampler = LowLvlGfx::Create(standardDescriptors::g_sample_anisotropic_wrap);
	m_anisotropicClampSampler = LowLvlGfx::Create(standardDescriptors::g_sample_anisotropic_clamp);
	m_pointWrapSampler = LowLvlGfx::Create(standardDescriptors::g_sample_point_wrap);
	m_pointClampSampler = LowLvlGfx::Create(standardDescriptors::g_sample_point_clamp);

	D3D11_SAMPLER_DESC samplerDesc = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
		.AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
		.AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {1,1,1,1},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};

	m_shadowMapSampler = LowLvlGfx::Create(samplerDesc);
}

void PbrRenderer::SetDiffuseIrradianceCubeMap(std::shared_ptr<Texture2D> irrMap)
{
	m_irradSkyMap = irrMap;
}

void PbrRenderer::SetSplitSumAproxLookUpMap(std::shared_ptr<Texture2D> splitSumLUMap)
{
	m_splitSumLookUpMap = splitSumLUMap;
}

void PbrRenderer::SetSpecularCubeMap(std::shared_ptr<Texture2D> specMap)
{
	m_specCubeMap = specMap;
}

void PbrRenderer::Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialVariantEnum type)
{
	if (type == MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR)
	{
		m_PBR_ALBEDO_METROUG_NOR.emplace_back(unitID, worlMatrix, type);
	}
	else if (type == MaterialVariantEnum::PBR_ALBEDO_METROUG)
	{
		m_PBR_ALBEDO_METROUG.emplace_back(unitID, worlMatrix, type);
	}
	else if (type == MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR_EMIS)
	{
		m_PBR_ALBEDO_METROUG_NOR_EMIS.emplace_back(unitID, worlMatrix, type);
	}
	else if (type == MaterialVariantEnum::PBR_NO_TEXTURES)
	{
		m_PBR_NO_TEXTURES.emplace_back(unitID, worlMatrix, type);
	}
	else if (type == MaterialVariantEnum::PBR_ALBEDO)
	{
		m_PBR_ALBEDO.emplace_back(unitID, worlMatrix, type);
	}
	else if (type == MaterialVariantEnum::PBR_ALBEDO_NOR)
	{
		m_PBR_ALBEDO_NOR.emplace_back(unitID, worlMatrix, type);
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

	//move binds to some other function that will not be called multiple times per frame
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	LowLvlGfx::Bind(rendRes->m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(rendRes->m_dirLightCB, ShaderType::PIXELSHADER, 3);
	LowLvlGfx::Bind(rendRes->m_shadowMapViewProjCB, ShaderType::PIXELSHADER, 4);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(rendRes->m_vpCB, ShaderType::PIXELSHADER, 1);
	
	
	LowLvlGfx::Bind(m_linearWrapSampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(m_linearClampSamplerDefault, ShaderType::PIXELSHADER, 2);
	LowLvlGfx::Bind(m_shadowMapSampler, ShaderType::PIXELSHADER, 4);
	LowLvlGfx::BindSRV(m_splitSumLookUpMap, ShaderType::PIXELSHADER, 4);
	LowLvlGfx::BindSRV(m_specCubeMap, ShaderType::PIXELSHADER, 5);
	LowLvlGfx::BindSRV(m_irradSkyMap, ShaderType::PIXELSHADER, 6);
	LowLvlGfx::BindSRV(rendRes->shadowMap, ShaderType::PIXELSHADER, 7);

	
	HandleRenderFlag(flag);

	RenderPBR_ALBEDO_METROUG_NOR_EMIS(flag);
	RenderPBR_ALBEDO_METROUG_NOR(flag);
	RenderPBR_ALBEDO_METROUG(flag);
	RenderPBR_NO_TEXTURES(flag);
	RenderPBR_ALBEDO_NOR(flag);
	RenderPBR_ALBEDO(flag);

	LowLvlGfx::UnBindSRV(ShaderType::PIXELSHADER, 7);

	m_prePocessed = false;
}

void PbrRenderer::DrawGeometry(const Mesh& mesh, ConstantBuffer& cb, const void* transform)
{
	LowLvlGfx::UpdateBuffer(cb, transform);
	LowLvlGfx::Bind(cb, ShaderType::VERTEXSHADER, 0);
	LowLvlGfx::Bind(mesh.vb);
	LowLvlGfx::Bind(mesh.ib);
	LowLvlGfx::DrawIndexed(mesh.GetIndexCount(), mesh.GetStartIndexLocation(), mesh.GetbaseVertexLocation());
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
		const auto& mat = rendUnit.material;
		auto albedoTex = assetMan.GetTexture2D(mat.baseColorTexture);
		auto normalTex = assetMan.GetTexture2D(mat.normalTexture);
		auto matallicRoughnessText = assetMan.GetTexture2D(mat.metallicRoughnessTexture);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::BindSRV(normalTex, ShaderType::PIXELSHADER, 2);

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
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
		const auto& mat = rendUnit.material;
		auto albedoTex = assetMan.GetTexture2D(mat.baseColorTexture);
		auto matallicRoughnessText = assetMan.GetTexture2D(mat.metallicRoughnessTexture);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
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
		const auto& mat = rendUnit.material;
		auto albedoTex = assetMan.GetTexture2D(mat.baseColorTexture);
		auto normalTex = assetMan.GetTexture2D(mat.normalTexture);
		auto matallicRoughnessText = assetMan.GetTexture2D(mat.metallicRoughnessTexture);
		auto emissiveText = assetMan.GetTexture2D(mat.emissiveTexture);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(matallicRoughnessText, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::BindSRV(normalTex, ShaderType::PIXELSHADER, 2);
		LowLvlGfx::BindSRV(emissiveText, ShaderType::PIXELSHADER, 3);

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
	}

	m_PBR_ALBEDO_METROUG_NOR_EMIS.clear();
}

void PbrRenderer::RenderPBR_NO_TEXTURES(RenderFlag flag)
{
	if (m_PBR_NO_TEXTURES.empty()) return;

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShader);

	if ((flag & RenderFlag::pixel_shader_terrain) != 0)
		LowLvlGfx::Bind(m_PS_terrainShading);
	else
		LowLvlGfx::Bind(m_PS_PBR_NOTEXTURES);
		
	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());


	for (auto& unit : m_PBR_NO_TEXTURES)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		const auto& mat = rendUnit.material;

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
	}

	m_PBR_NO_TEXTURES.clear();
}

void PbrRenderer::RenderPBR_ALBEDO(RenderFlag flag)
{
	if (m_PBR_ALBEDO.empty()) return;

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShader);
	LowLvlGfx::Bind(m_PS_PBR_AL);

	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());

	for (auto& unit : m_PBR_ALBEDO)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		const auto& mat = rendUnit.material;
		auto albedoTex = assetMan.GetTexture2D(mat.baseColorTexture);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
	}

	m_PBR_ALBEDO.clear();
}

void PbrRenderer::RenderPBR_ALBEDO_NOR(RenderFlag flag)
{
	if (m_PBR_ALBEDO_NOR.empty()) return;

	auto rendRes = m_sharedRenderResources.lock();
	const AssetManager& assetMan = AssetManager::Get();
	LowLvlGfx::Bind(rendRes->m_vertexShaderNormalMap);
	LowLvlGfx::Bind(m_PS_PBR_AL_NOR);

	//LowLvlGfx::BindRTVs({ rendRes->m_hdrRenderTarget }, LowLvlGfx::GetDepthBuffer());

	for (auto& unit : m_PBR_ALBEDO_NOR)
	{
		const RenderUnit& rendUnit = assetMan.GetRenderUnit(unit.id);
		const auto& mat = rendUnit.material;
		auto albedoTex = assetMan.GetTexture2D(mat.baseColorTexture);
		auto normalTex = assetMan.GetTexture2D(mat.normalTexture);

		LowLvlGfx::BindSRV(albedoTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::BindSRV(normalTex, ShaderType::PIXELSHADER, 2);

		PbrMaterialCBStruct cMat;
		cMat.albedoFactor = mat.baseColorFactor;
		cMat.roughnessFactor = mat.roughnessFactor;
		cMat.metallicFactor = mat.metallicFactor;
		cMat.emissiveFactor = mat.emissiveFactor;
		LowLvlGfx::UpdateBuffer(m_pbrCB, &cMat);
		LowLvlGfx::Bind(m_pbrCB, ShaderType::PIXELSHADER, 2);

		DrawGeometry(rendUnit.GetMesh(), rendRes->m_worldMatrixCB, &unit.worldMatrix);
	}

	m_PBR_ALBEDO_NOR.clear();
}

void PbrRenderer::HandleRenderFlag(RenderFlag flag)
{

	if (RenderFlag::none == flag)
	{
		LowLvlGfx::UnBindBlendState();
		LowLvlGfx::UnBindRasterizer();
		LowLvlGfx::Bind(m_linearClampSamplerDefault, ShaderType::PIXELSHADER, 3);
	}
	if ((flag & RenderFlag::alphaToCov) != 0)
	{
		LowLvlGfx::Bind(m_alphaToCovBlend);
	}
	else if ((flag & RenderFlag::alphaBlend) != 0)
	{
		LowLvlGfx::Bind(m_BlendState);
	}
	else
	{
		LowLvlGfx::UnBindBlendState();
	}

	if (((flag & RenderFlag::noBackFaceCull) != 0) && (flag & RenderFlag::wireframe) != 0)
	{
		LowLvlGfx::Bind(m_noBackFaceCullAndWireframeRasterizer);
	}
	else if ((flag & RenderFlag::noBackFaceCull) != 0)
	{
		LowLvlGfx::Bind(m_noBackFaceCullRasterizer);
	}
	else if ((flag & RenderFlag::wireframe) != 0)
	{
		LowLvlGfx::Bind(m_wireframeRasterizer);
	}
	else
	{
		LowLvlGfx::UnBindRasterizer();
	}

	if ((flag & RenderFlag::sampler_linear_clamp) != 0)
	{
		LowLvlGfx::Bind(m_linearClampSamplerDefault, ShaderType::PIXELSHADER, 3);
	}
	else if ((flag & RenderFlag::sampler_linear_wrap) != 0)
	{
		LowLvlGfx::Bind(m_linearWrapSampler, ShaderType::PIXELSHADER, 3);
	}
	else if ((flag & RenderFlag::sampler_anisotropic_wrap) != 0)
	{
		LowLvlGfx::Bind(m_anisotropicWrapSampler, ShaderType::PIXELSHADER, 3);
	}
	else if ((flag & RenderFlag::sampler_anisotropic_clamp) != 0)
	{
		LowLvlGfx::Bind(m_anisotropicClampSampler, ShaderType::PIXELSHADER, 3);
	}
	else if ((flag & RenderFlag::sampler_point_wrap) != 0)
	{
		LowLvlGfx::Bind(m_pointWrapSampler, ShaderType::PIXELSHADER, 3);
	}
	else if ((flag & RenderFlag::sampler_point_clamp) != 0)
	{
		LowLvlGfx::Bind(m_pointClampSampler, ShaderType::PIXELSHADER, 3);
	}


}
