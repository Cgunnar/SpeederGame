#include "pch.hpp"
#include "Material.h"
#include "AssetManager.h"

MaterialVariant::MaterialVariant(const Material& pbrMaterial)
{
	AssetManager& am = AssetManager::Get();
	this->name = pbrMaterial.name;
	if (pbrMaterial.normalTexture &&
		pbrMaterial.baseColorTexture &&
		pbrMaterial.metallicRoughnessTexture &&
		pbrMaterial.emissiveTexture)
	{
		PBR_ALBEDO_METROUG_NOR_EMIS mat;
		mat.albedoTextureID = pbrMaterial.baseColorTexture;
		mat.normalTextureID = pbrMaterial.normalTexture;
		mat.emissiveTextureID = pbrMaterial.emissiveTexture;
		mat.matallicRoughnessTextureID = pbrMaterial.metallicRoughnessTexture;

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR_EMIS;
	}
	else if (pbrMaterial.normalTexture &&
			pbrMaterial.baseColorTexture &&
			pbrMaterial.metallicRoughnessTexture)
	{
		PBR_ALBEDO_METROUG_NOR mat;
		mat.albedoTextureID = pbrMaterial.baseColorTexture;
		mat.normalTextureID = pbrMaterial.normalTexture;
		mat.matallicRoughnessTextureID = pbrMaterial.metallicRoughnessTexture;

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR;
	}
	else if (pbrMaterial.normalTexture && pbrMaterial.baseColorTexture)
	{
		PBR_ALBEDO_NOR mat;
		mat.albedoTextureID = pbrMaterial.baseColorTexture;
		mat.normalTextureID = pbrMaterial.normalTexture;

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_NOR;
	}
	else if (pbrMaterial.baseColorTexture && pbrMaterial.metallicRoughnessTexture)
	{
		PBR_ALBEDO_METROUG mat;
		mat.albedoTextureID = pbrMaterial.baseColorTexture;
		mat.matallicRoughnessTextureID = pbrMaterial.metallicRoughnessTexture;

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG;
	}
	else if (pbrMaterial.baseColorTexture)
	{
		PBR_ALBEDO mat;
		mat.albedoTextureID = pbrMaterial.baseColorTexture;
		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO;
	}
	else if (pbrMaterial.normalTexture)
	{
		assert(false);
	}
	else
	{
		PBR_NO_TEXTURES mat;
		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_NO_TEXTURES;
	}

	if (pbrMaterial.blendMode == BlendMode::blend)
	{
		this->renderFlag |= RenderFlag::alphaBlend;
	}
	else if (pbrMaterial.blendMode == BlendMode::mask)
	{
		this->renderFlag |= RenderFlag::alphaToCov;
	}

	if ((pbrMaterial.properties & MaterialProperties::NO_BACKFACE_CULLING) != 0)
	{
		this->renderFlag |= RenderFlag::noBackFaceCull;
	}
}

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
