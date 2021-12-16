#pragma once
#include "SharedRendererResources.h"
class PbrRenderer
{
public:
	PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes);
	PbrRenderer() = default;


	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type);
	void PreProcess(const VP& viewAndProjMatrix);
	void Render(const VP& viewAndProjMatrix);
private:
	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;

	Shader m_PS_PBR_AL_MERO_NO_PointLight;
	Shader m_PS_PBR_ALB_METROU_PointLight;

	bool m_prePocessed = false;
	void RenderPBR_ALBEDO_METROUG_NOR();
	void RenderPBR_ALBEDO_METROUG();

	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR;
	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG;
};

