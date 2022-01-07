#pragma once
#include "RimfrostMath.hpp"
#include "utilityTypes.h"
#include "GraphicsUtilityTypes.h"

enum class RenderFlag
{
	none = 0,
	wireframe = 1 << 1,
	noBackFaceCull = 1 << 2,
	alphaToCov = 1 << 3,
	alphaBlend = 1 << 4,
	sampler_wrap = 1 << 5,
	sampler_clamp = 1 << 6,
	end = 1 << 7,
};
inline RenderFlag operator &(RenderFlag l, RenderFlag r)
{
	return (RenderFlag)((int)l & (int)r);
}
inline RenderFlag operator |(RenderFlag l, RenderFlag r)
{
	return (RenderFlag)((int)l | (int)r);
}
inline bool operator != (RenderFlag l, int r)
{
	return (bool)((int)l != r);
}
inline bool operator == (RenderFlag l, int r)
{
	return (bool)((int)l == r);
}

inline void operator |= (RenderFlag& l, RenderFlag r)
{
	l = l | r;
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
	void SetBaseColorTexture(const std::string& path);
	void SetNormalTexture(const std::string& path);
	void SetMetallicRoughnessTexture(const std::string& path);
	void SetEmissiveTexture(const std::string& path);
	MaterialVariantEnum GetType() const;
	RenderFlag flags = RenderFlag::none;
	std::string name = "";
	GID baseColorTexture;
	GID normalTexture;
	GID metallicRoughnessTexture;
	GID emissiveTexture;

	float maskCutOfValue = 0;
	float metallicFactor = 0;
	float roughnessFactor = 1;
	rfm::Vector4 baseColorFactor = 1;
	rfm::Vector3 emissiveFactor = 0;
};
