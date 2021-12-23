#pragma once
#include <string>
#include <memory>

#include "GraphicsResources.h"
#include "SharedRendererResources.h"
class SkyBox
{
public:
	SkyBox() = default;
	void Init(const std::string& path);

	void Bind(SharedRenderResources& rendRes);
private:
	std::shared_ptr<Texture2D> m_skyBoxCubeMap;
	rfm::Matrix m_rotation;
	Shader m_skyBoxVS;
	Shader m_skyBoxPS;
	Rasterizer m_rasterizer;
};

