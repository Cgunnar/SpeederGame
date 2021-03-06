#pragma once
#include "GraphicsResources.h"
#include "RenderComponents.h"
#include "RimfrostMath.hpp"
#include "SharedRendererResources.h"
class ShadowMappingPass
{
public:
	ShadowMappingPass() = default;
	ShadowMappingPass(std::weak_ptr<SharedRenderResources> sharedRes, uint32_t res = 4096);
	void Bind(rfe::Entity camera, rfm::Vector3 lightDirection);
	void DrawFromDirLight(const std::vector<RendUnitIDAndTransform>& geometyToRender);
	const rfm::Matrix* GetViewProjectionMatrix() const;
private:
	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;
	uint32_t m_res = 4096;
	Shader m_vertexShader;
	Shader m_emptyPixelShader;
	rfm::Matrix m_vp;
	rfm::Matrix m_projectionMatrix;
};

