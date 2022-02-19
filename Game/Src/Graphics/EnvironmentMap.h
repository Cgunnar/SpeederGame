#pragma once
#include "GraphicsResources.h"

class EnvironmentMap
{
public:
	EnvironmentMap();
	EnvironmentMap(std::shared_ptr<Texture2D> cubeMap);
	std::shared_ptr<Texture2D> GetIrradianceCubeMap();
	std::shared_ptr<Texture2D> GetSpecularCubeMap();
private:
	std::shared_ptr<Texture2D> m_irradianceCubeMap = nullptr;
	std::shared_ptr<Texture2D> m_specularCubeMap = nullptr;
};

