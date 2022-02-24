#pragma once
#include <string>
#include <memory>

#include "GraphicsResources.h"
#include "SharedRendererResources.h"
#include "EnvironmentMap.h"
class Renderer;
class SkyBox
{
	friend Renderer;
public:
	SkyBox() = default;
	void Init(const std::string& path);
	bool Hdr() const;
	bool Ldr() const;
	void Bind(SharedRenderResources& rendRes);
	void SetRotation(rfm::Matrix rot);
	std::shared_ptr<Texture2D> GetSkyCubeMap();
private:
	void InitCubeMapLDR(const std::string& path);
	void InitCubeMapHDR(const std::string& path);

	std::shared_ptr<Texture2D> GenerateSky(uint32_t cubeSideLength, bool mipMapping);
	std::shared_ptr<Texture2D> LoadEquirectangularMapToCubeMap(const std::string& path, uint32_t cubeSideLength, bool mipMapping);

	bool m_ldr = false;
	bool m_hdr = false;
	std::shared_ptr<Texture2D> m_skyBoxCubeMap;

	EnvironmentMap m_envMap;
	
	rfm::Matrix m_rotation;
	Shader m_skyBoxVS;
	Shader m_skyBoxPS;

	Shader m_eq2cubeCS;
	/*Shader m_convolute_DiffIrrCubeCS;
	Shader m_spmapCS;*/
	
	Shader m_atmospheric_scatteringCS;

	ConstantBuffer m_roughnessCB;
	Rasterizer m_rasterizer;
	Sampler m_sampler;
};

