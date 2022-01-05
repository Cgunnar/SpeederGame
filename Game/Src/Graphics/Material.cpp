#include "pch.hpp"
#include "Material.h"
#include "AssetManager.h"

MaterialVariant::MaterialVariant(const Material& pbrMaterial)
{
	AssetManager& am = AssetManager::Get();
	this->materialVariant = pbrMaterial;
	this->name = pbrMaterial.name;
	if (pbrMaterial.normalTexture && pbrMaterial.baseColorTexture &&
		pbrMaterial.metallicRoughnessTexture && pbrMaterial.emissiveTexture)
	{
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR_EMIS;
	}
	else if (pbrMaterial.normalTexture && pbrMaterial.baseColorTexture &&
			pbrMaterial.metallicRoughnessTexture)
	{
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG_NOR;
	}
	else if (pbrMaterial.normalTexture && pbrMaterial.baseColorTexture)
	{
		this->type = MaterialVariantEnum::PBR_ALBEDO_NOR;
	}
	else if (pbrMaterial.baseColorTexture && pbrMaterial.metallicRoughnessTexture)
	{
		this->type = MaterialVariantEnum::PBR_ALBEDO_METROUG;
	}
	else if (pbrMaterial.baseColorTexture)
	{
		this->type = MaterialVariantEnum::PBR_ALBEDO;
	}
	else if (pbrMaterial.normalTexture)
	{
		assert(false);
	}
	else
	{
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

	if ((pbrMaterial.properties & MaterialProperties::WIREFRAME) != 0)
	{
		this->renderFlag |= RenderFlag::wireframe;
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
