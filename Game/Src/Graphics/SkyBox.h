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
	std::shared_ptr<Texture2D> ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap);
	std::shared_ptr<Texture2D> ConvoluteSpecularCubeMap(std::shared_ptr<Texture2D> envMap);
	std::shared_ptr<Texture2D> LoadEquirectangularMapToCubeMap(const std::string& path, uint32_t cubeSideLength, bool mipMapping);
	bool m_ldr = false;
	bool m_hdr = false;
	std::shared_ptr<Texture2D> m_skyBoxCubeMap;
	std::shared_ptr<Texture2D> m_irradianceCubeMap;
	std::shared_ptr<Texture2D> m_specularCubeMap;
	rfm::Matrix m_rotation;
	Shader m_skyBoxVS;
	Shader m_skyBoxPS;

	Shader m_eq2cubeCS;
	Shader m_convolute_DiffIrrCubeCS;
	Shader m_spbrdfCS;
	Shader m_spmapCS;

	ConstantBuffer m_roughnessCB;
	Rasterizer m_rasterizer;
	Sampler m_sampler;
};

