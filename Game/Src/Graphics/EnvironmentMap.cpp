#include "pch.hpp"
#include "EnvironmentMap.h"

EnvironmentMap::EnvironmentMap()
{

}

EnvironmentMap::EnvironmentMap(std::shared_ptr<Texture2D> cubeMap)
{
	/*m_irradianceCubeMap = irrMap;
	m_specularCubeMap = specMap;*/
}

std::shared_ptr<Texture2D> EnvironmentMap::GetIrradianceCubeMap()
{
	return m_irradianceCubeMap;
}

std::shared_ptr<Texture2D> EnvironmentMap::GetSpecularCubeMap()
{
	return m_specularCubeMap;
}
