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
	bool Hdr() const;
	bool Ldr() const;
	void Bind(SharedRenderResources& rendRes);
	void SetRotation(rfm::Matrix rot);
private:
	void InitCubeMapLDR(const std::string& path);
	void InitCubeMapHDR(const std::string& path);
	bool m_ldr = false;
	bool m_hdr = false;
	std::shared_ptr<Texture2D> m_skyBoxCubeMap;
	rfm::Matrix m_rotation;
	Shader m_skyBoxVS;
	Shader m_skyBoxPS;

	Shader m_eq2cubeCS;

	Rasterizer m_rasterizer;
	Sampler m_sampler;
};

