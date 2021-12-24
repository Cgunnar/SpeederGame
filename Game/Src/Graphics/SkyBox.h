#pragma once
#include <string>
#include <memory>

#include "GraphicsResources.h"
#include "SharedRendererResources.h"
class Renderer;
class SkyBox
{
	friend Renderer;
public:
	SkyBox() = default;
	void Init(const std::string& path, const std::string& irradianceMapPath = "");
	bool Hdr() const;
	bool Ldr() const;
	void Bind(SharedRenderResources& rendRes);
	void SetRotation(rfm::Matrix rot);
private:
	void InitCubeMapLDR(const std::string& path);
	void InitCubeMapHDR(const std::string& path, const std::string& irradianceMapPath);
	std::shared_ptr<Texture2D> LoadEquirectangularMapToCubeMap(const std::string& path, uint32_t cubeSideLength = 1024);
	bool m_ldr = false;
	bool m_hdr = false;
	std::shared_ptr<Texture2D> m_skyBoxCubeMap;
	std::shared_ptr<Texture2D> m_irradianceCubeMap;
	rfm::Matrix m_rotation;
	Shader m_skyBoxVS;
	Shader m_skyBoxPS;

	Shader m_eq2cubeCS;

	Rasterizer m_rasterizer;
	Sampler m_sampler;
};

