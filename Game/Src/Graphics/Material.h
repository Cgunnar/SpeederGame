#pragma once
#include <variant>
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
	DIFFUSE_MAP = 1 << 0,
	SPECULAR_MAP = 1 << 1,
	NORMAL_MAP = 1 << 2,
	SHININESS = 1 << 3,
	DIFFUSE_COLOR = 1 << 4,
	SPECULAR_COLOR = 1 << 5,
	AMBIENT_COLOR = 1 << 6,

	ALPHA_BLENDING = 1 << 7,
	ALPHA_BLENDING_CONSTANS_OPACITY = 1 << 8,
	ALPHA_TESTING = 1 << 9,

	IS_EMISSIVE = 1 << 10,
	METALLICROUGHNESS = 1 << 11,
	ALBEDO_MAP = 1 << 12,
	PBR = 1 << 13,
	NO_BACKFACE_CULLING = 1 << 14,
	WIREFRAME = 1 << 15,

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
	PBR_ALBEDO = 1 << 7,
	PBR_NO_TEXTURES = 1 << 8,
};
inline MaterialType operator &(MaterialType l, MaterialType r)
{
	return (MaterialType)((int)l & (int)r);
}
inline MaterialType operator |(MaterialType l, MaterialType r)
{
	return (MaterialType)((int)l | (int)r);
}

inline void operator |= (MaterialType& l, MaterialType r)
{
	l = l | r;
}


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
	rfm::Vector3 emissiveFactor = rfm::Vector4(1, 1, 1, 1);
};

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
	MaterialType type = MaterialType::none;
	RenderFlag renderFlag = RenderFlag::none;

	std::variant <
		PhongMaterial_Color, PhongMaterial_DiffTex, PhongMaterial_DiffTex_NormTex, PhongMaterial_DiffTex_NormTex_SpecTex,
		PBR_ALBEDO_METROUG_NOR, PBR_ALBEDO_METROUG, PBR_ALBEDO_METROUG_NOR_EMIS, PBR_ALBEDO, PBR_NO_TEXTURES
	> materialVariant;
};

