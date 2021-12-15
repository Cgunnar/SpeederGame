#pragma once
#include <variant>
#include "RimfrostMath.hpp"
#include "utilityTypes.h"


struct PhongMaterial_Color
{
	//linear color space
	rfm::Vector3 ambientColor{ 0.8f, 0.8f, 0.8f };
	rfm::Vector3 diffuseColor{ 0.8f, 0.8f, 0.8f };
	rfm::Vector3 specularColor{ 1, 1, 1 };
	float shininess = 800;
};

struct PhongMaterial_DiffTex
{
	GID diffuseTextureID;
	rfm::Vector3 specularColor{ 1, 1, 1 };
	float shininess = 800;
};

struct PhongMaterial_DiffTex_NormTex
{
	GID diffuseTextureID;
	GID normalTextureID;
	rfm::Vector3 specularColor{ 1, 1, 1 };
	float shininess = 800;
};

struct PBR_ALBEDO_METROUG_NOR
{
	GID albedoTextureID;
	GID normalTextureID;
	GID matallicRoughnessTextureID;
};


enum class MaterialType
{
	none = 0,
	PhongMaterial_Color = 1 << 0,
	PhongMaterial_DiffTex = 1 << 1,
	PhongMaterial_DiffTex_NormTex = 1 << 2,
	PBR_ALBEDO_METROUG_NOR = 1 << 3,
	NormalMap = 1 << 4,
	wireframe = 1 << 5,
	transparent = 1 << 6,
};
inline MaterialType operator &(MaterialType l, MaterialType r)
{
	return (MaterialType)((int)l & (int)r);
}
inline MaterialType operator |(MaterialType l, MaterialType r)
{
	return (MaterialType)((int)l | (int)r);
}

struct Material
{
	std::string name;
	MaterialType type = MaterialType::none;

	std::variant <
		PhongMaterial_Color, PhongMaterial_DiffTex, PhongMaterial_DiffTex_NormTex, PBR_ALBEDO_METROUG_NOR
	> materialVariant;
};

