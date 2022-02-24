#pragma once
#include "GraphicsResources.h"

class EnvironmentMap
{
public:
	EnvironmentMap();
	EnvironmentMap(uint32_t sideLength);
	EnvironmentMap(std::shared_ptr<Texture2D> cubeMap);
	void UpdateEnvMap(std::shared_ptr<Texture2D> cubeMap);
	std::shared_ptr<Texture2D> GetIrradianceCubeMap();
	std::shared_ptr<Texture2D> GetSpecularCubeMap();
	bool IsValid();
private:
	void ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> irrMapOut);
	void ConvoluteSpecularCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> specMapOut);
	bool m_isValid = false;
	std::shared_ptr<Texture2D> m_irradianceCubeMap = nullptr;
	std::shared_ptr<Texture2D> m_specularCubeMap = nullptr;

	static Shader s_convolute_DiffIrrCubeCS;
	static Shader s_spmapCS;
	ConstantBuffer m_roughnessCB; //static?
};

