#pragma once
#include "GraphicsResources.h"
#include "RenderComponents.h"
#include "RimfrostMath.hpp"
class ShadowMappingPass
{
public:
	ShadowMappingPass(uint32_t res = 4096);
	void DrawFromDirLight(rfm::Vector3 lightDirection, const std::vector<RendCompAndTransform>& geometyToRender);

private:
	void Draw(const SubMesh& mesh, const rfm::Matrix& worldMatrix);
	uint32_t m_res;
	Shader m_vertexShader;
	Shader m_emptyPixelShader;
	std::shared_ptr<Texture2D> m_shadowMap;
	VP m_vp;
};

