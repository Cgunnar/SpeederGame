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

struct PhongMaterial_DiffTex_NormTex_SpecTex
{
	GID diffuseTextureID;
	GID normalTextureID;
	GID specularTextureID;
	float shininess = 800;
};

struct PBR_ALBEDO_METROUG_NOR
{
	GID albedoTextureID;
	GID normalTextureID;
	GID matallicRoughnessTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};

struct PBR_ALBEDO_METROUG_NOR_EMIS
{
	GID albedoTextureID;
	GID normalTextureID;
	GID matallicRoughnessTextureID;
	GID emissiveTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};

struct PBR_ALBEDO_METROUG
{
	GID albedoTextureID;
	GID matallicRoughnessTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};

struct PBR_NO_TEXTURES
{
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};


enum class MaterialType
{
	none = 0,
	PhongMaterial_Color = 1 << 0,
	PhongMaterial_DiffTex = 1 << 1,
	PhongMaterial_DiffTex_NormTex = 1 << 2,
	PhongMaterial_DiffTex_NormTex_SpecTex = 1 << 3,
	PBR_ALBEDO_METROUG_NOR = 1 << 4,
	PBR_ALBEDO_METROUG = 1 << 5,
	PBR_ALBEDO_METROUG_NOR_EMIS = 1 << 6,
	PBR_NO_TEXTURES = 1 << 7,
	wireframe = 1 << 8,
	transparent = 1 << 9,
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
		PhongMaterial_Color, PhongMaterial_DiffTex, PhongMaterial_DiffTex_NormTex, PhongMaterial_DiffTex_NormTex_SpecTex,
		PBR_ALBEDO_METROUG_NOR, PBR_ALBEDO_METROUG, PBR_ALBEDO_METROUG_NOR_EMIS, PBR_NO_TEXTURES
	> materialVariant;
};

