#include "pch.hpp"
#include "Material.h"
#include "AssetManager.h"

MaterialVariant::MaterialVariant(const Material& pbrMaterial)
{
	AssetManager& am = AssetManager::Get();
	this->name = pbrMaterial.name;
	if (!pbrMaterial.normalPath.empty() &&
		!pbrMaterial.baseColorPath.empty() &&
		!pbrMaterial.metallicRoughnessPath.empty() &&
		!pbrMaterial.emissivePath.empty())
	{
		PBR_ALBEDO_METROUG_NOR_EMIS mat;
		mat.albedoTextureID = am.LoadTex2D(pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
		mat.normalTextureID = am.LoadTex2D(pbrMaterial.normalPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
		mat.emissiveTextureID = am.LoadTex2D(pbrMaterial.emissivePath, LoadTexFlag::GenerateMips);
		mat.matallicRoughnessTextureID = am.LoadTex2D(pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR_EMIS;
	}
	else if (!pbrMaterial.normalPath.empty() &&
			!pbrMaterial.baseColorPath.empty() &&
			!pbrMaterial.metallicRoughnessPath.empty())
	{
		PBR_ALBEDO_METROUG_NOR mat;
		mat.albedoTextureID = am.LoadTex2D(pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
		mat.normalTextureID = am.LoadTex2D(pbrMaterial.normalPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
		mat.matallicRoughnessTextureID = am.LoadTex2D(pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR;
	}
	else if (!pbrMaterial.normalPath.empty() && !pbrMaterial.baseColorPath.empty())
	{
		PBR_ALBEDO_NOR mat;
		mat.albedoTextureID = am.LoadTex2D(pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
		mat.normalTextureID = am.LoadTex2D(pbrMaterial.normalPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_NOR;
	}
	else if (!pbrMaterial.baseColorPath.empty() && !pbrMaterial.metallicRoughnessPath.empty())
	{
		PBR_ALBEDO_METROUG mat;
		mat.albedoTextureID = am.LoadTex2D(pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
		mat.matallicRoughnessTextureID = am.LoadTex2D(pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG;
	}
	else if (!pbrMaterial.baseColorPath.empty())
	{
		PBR_ALBEDO mat;
		mat.albedoTextureID = am.LoadTex2D(pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);

		mat.emissiveFactor = pbrMaterial.emissiveFactor;
		mat.rgba = pbrMaterial.baseColorFactor;
		mat.metallic = pbrMaterial.metallicFactor;
		mat.roughness = pbrMaterial.roughnessFactor;

		this->materialVariant = mat;
		this->type = MaterialVariantEnum::PBR_ALBEDO;
	}
	else if (!pbrMaterial.normalPath.empty())
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
