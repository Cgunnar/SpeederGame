#pragma once
#include "SharedRendererResources.h"
class PbrRenderer
{
public:
	PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes);


	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type);
	void PreProcess(const VP& viewAndProjMatrix);
	void Render(const VP& viewAndProjMatrix);
private:
	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;

	bool m_prePocessed = false;


	std::vector<RendUnitIDAndTransform> m_PBR_ALBEDO_METROUG_NOR;
};

