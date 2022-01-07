#include "pch.hpp"
#include "Material.h"
#include "AssetManager.h"

void Material::SetBaseColorTexture(const std::string& path)
{
	baseColorTexture = AssetManager::Get().LoadTex2DFromFile(path, LoadTexFlag::GenerateMips);
}

void Material::SetNormalTexture(const std::string& path)
{
	normalTexture = AssetManager::Get().LoadTex2DFromFile(path, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
}

void Material::SetMetallicRoughnessTexture(const std::string& path)
{
	metallicRoughnessTexture = AssetManager::Get().LoadTex2DFromFile(path, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
}

void Material::SetEmissiveTexture(const std::string& path)
{
	emissiveTexture = AssetManager::Get().LoadTex2DFromFile(path, LoadTexFlag::GenerateMips);
}

MaterialVariantEnum Material::GetType() const
{
	if (normalTexture && baseColorTexture && metallicRoughnessTexture && emissiveTexture)
	{
		return MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR_EMIS;
	}
	if (normalTexture && baseColorTexture && metallicRoughnessTexture)
	{
		return  MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR;
	}
	if (normalTexture && baseColorTexture)
	{
		return MaterialVariantEnum::PBR_ALBEDO_NOR;
	}
	if (baseColorTexture && metallicRoughnessTexture)
	{
		return MaterialVariantEnum::PBR_ALBEDO_METROUG;
	}
	if (baseColorTexture)
	{
		return MaterialVariantEnum::PBR_ALBEDO;
	}
	if (normalTexture)
	{
		assert(false);
	}
	return MaterialVariantEnum::PBR_NO_TEXTURES;
}

