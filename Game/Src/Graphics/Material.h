#pragma once
#include "RimfrostMath.hpp"
#include "utilityTypes.h"
#include "GraphicsUtilityTypes.h"

enum class BlendMode
{
	opaque = 0,
	blend,
	mask,
};

enum class MaterialProperties
{
	NONE = 0,
	NO_BACKFACE_CULLING = 1 << 0,
	WIREFRAME = 1 << 1,

};

inline MaterialProperties operator &(MaterialProperties l, MaterialProperties r)
{
	return (MaterialProperties)((int)l & (int)r);
}
inline MaterialProperties operator |(MaterialProperties l, MaterialProperties r)
{
	return (MaterialProperties)((int)l | (int)r);
}
inline bool operator != (MaterialProperties l, int r)
{
	return (bool)((int)l != r);
}



enum class MaterialVariantEnum
{
	none = 0,
	PBR_ALBEDO_METROUG_NOR,
	PBR_ALBEDO_METROUG,
	PBR_ALBEDO_METROUG_NOR_EMIS,
	PBR_ALBEDO,
	PBR_ALBEDO_NOR,
	PBR_NO_TEXTURES,
};


struct Material
{
	MaterialProperties properties = MaterialProperties::NONE;
	BlendMode blendMode = BlendMode::opaque;
	float maskCutOfValue = 0;
	std::string name = "";
	std::string baseColorPath = "";
	std::string normalPath = "";
	std::string metallicRoughnessPath = "";
	std::string emissivePath = "";

	float metallicFactor = 0;
	float roughnessFactor = 1;
	rfm::Vector4 baseColorFactor = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};

struct PBR_ALBEDO_METROUG_NOR
{
	GID albedoTextureID;
	GID normalTextureID;
	GID matallicRoughnessTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor;
};

struct PBR_ALBEDO_METROUG_NOR_EMIS
{
	GID albedoTextureID;
	GID normalTextureID;
	GID matallicRoughnessTextureID;
	GID emissiveTextureID;
	float metallic = 1;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor = rfm::Vector3(1, 1, 1);
};

struct PBR_ALBEDO_METROUG
{
	GID albedoTextureID;
	GID matallicRoughnessTextureID;
	float metallic = 1;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor;
};

struct PBR_ALBEDO_NOR
{
	GID albedoTextureID;
	GID normalTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor;
};

struct PBR_ALBEDO
{
	GID albedoTextureID;
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor;
};

struct PBR_NO_TEXTURES
{
	float metallic = 0;
	float roughness = 1;
	rfm::Vector4 rgba = rfm::Vector4(1, 1, 1, 1);
	rfm::Vector3 emissiveFactor;
};


struct MaterialVariant
{
	MaterialVariant() = default;
	MaterialVariant(const Material& pbrMaterial);
	std::string name;
	MaterialVariantEnum type = MaterialVariantEnum::none;
	RenderFlag renderFlag = RenderFlag::none;

	std::variant <PBR_ALBEDO_METROUG_NOR, PBR_ALBEDO_METROUG, PBR_ALBEDO_METROUG_NOR_EMIS,
		PBR_ALBEDO, PBR_ALBEDO_NOR, PBR_NO_TEXTURES
	> materialVariant;
};

