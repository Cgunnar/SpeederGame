#pragma once
#include "GraphicsResources.h"

class EnvironmentMap
{
public:
	enum class State
	{
		invalid = 0,
		safe_to_read,
		safe_to_update,
		writing_diff,
		writing_spec
	};
	EnvironmentMap();
	EnvironmentMap(uint32_t sideLength);
	EnvironmentMap(std::shared_ptr<Texture2D> cubeMap);
	State UpdateEnvMap(std::shared_ptr<Texture2D> cubeMap);
	std::shared_ptr<Texture2D> GetIrradianceCubeMap();
	std::shared_ptr<Texture2D> GetSpecularCubeMap();
	bool IsValid();
	State GetState();
private:
	void ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> irrMapOut);
	void ConvoluteSpecularCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> specMapOut);
	bool m_isValid = false;
	State m_state = State::invalid;
	uint64_t m_frameLastUpdated;
	std::shared_ptr<Texture2D> m_irradianceCubeMap = nullptr;
	std::shared_ptr<Texture2D> m_specularCubeMap = nullptr;
	std::shared_ptr<Texture2D> m_tempSourceToUpdateFrom = nullptr;

	static Shader s_convolute_DiffIrrCubeCS;
	static Shader s_spmapCS;
	ConstantBuffer m_roughnessCB; //static?
};

