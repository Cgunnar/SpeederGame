#pragma once
#include "SharedRendererResources.h"
#include "rfEntity.hpp"
class PbrRenderer
{
public:
	PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes);
	PbrRenderer() = default;

	void SetDiffuseIrradianceCubeMap(std::shared_ptr<Texture2D> irrMap);
	void SetSpecularCubeMap(std::shared_ptr<Texture2D> specMap);
	void SetSplitSumAproxLookUpMap(std::shared_ptr<Texture2D> splitSumLUMap);
	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialVariantEnum type);
	void PreProcess(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);
	void Render(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);
private:
	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;
	std::shared_ptr<Texture2D> m_irradSkyMap;
	std::shared_ptr<Texture2D> m_specCubeMap;
	std::shared_ptr<Texture2D> m_splitSumLookUpMap;

	Sampler m_linearClampSamplerDefault;
	Sampler m_linearWrapSampler;
	Sampler m_shadowMapSampler;
	Sampler m_anisotropicWrapSampler;
	Sampler m_anisotropicClampSampler;
	Sampler m_pointWrapSampler;
	Sampler m_pointClampSampler;
	ConstantBuffer m_pbrCB;
	Rasterizer m_noBackFaceCullRasterizer;
	Rasterizer m_noBackFaceCullAndWireframeRasterizer;
	Rasterizer m_wireframeRasterizer;
	BlendState m_alphaToCovBlend;
	BlendState m_BlendState;
	Shader m_PS_PBR_AL_MERO_NO_PointLight;
	Shader m_PS_PBR_ALB_METROU_PointLight;
	Shader m_PS_PBR_NOR_EMIS_PointLight;
	Shader m_PS_PBR_AL;
	Shader m_PS_PBR_NOTEXTURES;
	Shader m_PS_PBR_AL_NOR;
	Shader m_PS_terrainShading;

	bool m_prePocessed = false;
	void DrawGeometry(const Mesh& mesh, ConstantBuffer& cb, const void* transform);
	void RenderPBR_ALBEDO_METROUG_NOR(RenderFlag flag);
	void RenderPBR_ALBEDO_METROUG(RenderFlag flag);
	void RenderPBR_ALBEDO_METROUG_NOR_EMIS(RenderFlag flag);
	void RenderPBR_NO_TEXTURES(RenderFlag flag);
	void RenderPBR_ALBEDO(RenderFlag flag);
	void RenderPBR_ALBEDO_NOR(RenderFlag flag);

	void HandleRenderFlag(RenderFlag flag);

	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR_EMIS;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG;
	std::vector<RendUnitIDAndTransform> m_PBR_NO_TEXTURES;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_NOR;
};

