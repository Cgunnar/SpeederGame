#include "pch.hpp"
#include "PbrRenderer.h"

PbrRenderer::PbrRenderer(std::weak_ptr<SharedRenderResources> sharedRes)
{

}

void PbrRenderer::Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type)
{
	m_PBR_ALBEDO_METROUG_NOR.emplace_back(unitID, worlMatrix, type);
}

void PbrRenderer::PreProcess(const VP& viewAndProjMatrix)
{
	m_prePocessed = true;
}

void PbrRenderer::Render(const VP& viewAndProjMatrix)
{
	if (!m_prePocessed) PreProcess(viewAndProjMatrix);






	m_prePocessed = false;
}
