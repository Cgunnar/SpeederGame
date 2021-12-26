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
	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type);
	void PreProcess(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);
	void Render(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);
private:
	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;
	std::shared_ptr<Texture2D> m_irradSkyMap;
	std::shared_ptr<Texture2D> m_specCubeMap;

	ConstantBuffer m_pbrCB;
	Rasterizer m_noBackFaceCullRasterizer;
	BlendState m_alphaToCovBlend;
	BlendState m_BlendState;
	Shader m_PS_PBR_AL_MERO_NO_PointLight;
	Shader m_PS_PBR_ALB_METROU_PointLight;
	Shader m_PS_PBR_NOR_EMIS_PointLight;
	Shader m_PS_PBR;

	bool m_prePocessed = false;
	void RenderPBR_ALBEDO_METROUG_NOR(RenderFlag flag);
	void RenderPBR_ALBEDO_METROUG(RenderFlag flag);
	void RenderPBR_ALBEDO_METROUG_NOR_EMIS(RenderFlag flag);
	void RenderPBR_NO_TEXTURES(RenderFlag flag);

	void HandleRenderFlag(RenderFlag flag);

	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR_EMIS;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG;
	std::vector<RendUnitIDAndTransform> m_PBR_NO_TEXTURES;
};

