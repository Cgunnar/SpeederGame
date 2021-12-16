#pragma once
#include <memory>
#include "GraphicsResources.h"

struct SharedRenderResources : public std::enable_shared_from_this<SharedRenderResources>
{
	Sampler m_linearWrapSampler;
	ConstantBuffer m_worldMatrixCB;
	ConstantBuffer m_vpCB;
	Shader m_vertexShader;
	Shader m_vertexShaderNormalMap;
	ConstantBuffer m_pointLightCB;
	std::shared_ptr<Texture2D> m_hdrRenderTarget;
};