#include "pch.hpp"
#include "AssetManager.h"
#include "Geometry.h"
#include "LowLvlGfx.h"
#include "ReadImg.hpp"
#include "GraphicsHelperFunctions.h"
#include "Material.h"
AssetManager* AssetManager::s_instance = nullptr;



void AssetManager::Init()
{
	assert(!s_instance);
	s_instance = new AssetManager();
}

void AssetManager::Destroy()
{
	assert(s_instance);
	delete s_instance;
	s_instance = nullptr;
}

AssetManager::AssetManager()
{
	Geometry::Quad_POS_NOR_UV quad2;
	m_renderUnits.push_back(RenderUnit());
	SubMesh& mesh = m_renderUnits.back().subMesh;
	mesh.ib = LowLvlGfx::CreateIndexBuffer(quad2.IndexData(), quad2.indexCount);
	mesh.vb = LowLvlGfx::CreateVertexBuffer(quad2.VertexData(), quad2.arraySize, quad2.vertexStride);
	mesh.baseVertexLocation = 0;
	mesh.startIndexLocation = 0;
	mesh.indexCount = mesh.ib.GetIndexCount();

}

AssetManager::~AssetManager()
{
}

AssetManager& AssetManager::Get()
{
	assert(s_instance);
	return *s_instance;
}

std::shared_ptr<Texture2D> AssetManager::GetTexture2D(GID guid) const
{
	if (auto it = m_textures.find(guid); it != m_textures.end())
	{
		return it->second;
	}
	return nullptr;
}

const SubMesh& AssetManager::GetMesh(RenderUnitID id) const
{
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return m_renderUnits[id - 1].subMesh;
}

const SubMesh& AssetManager::GetMesh(SimpleMesh mesh) const
{
	RenderUnitID id = static_cast<RenderUnitID>(mesh);
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return m_renderUnits[id - 1].subMesh;
}

const RenderUnit& AssetManager::GetRenderUnit(RenderUnitID id) const
{
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return m_renderUnits[id - 1];
}


RenderUnitID AssetManager::AddMesh(SubMesh mesh)
{
	m_renderUnits.push_back({ mesh, Material() });
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(const SubMesh& subMesh, const Material& material)
{
	m_renderUnits.push_back({ subMesh, material });
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(RenderUnit renderUnit)
{
	m_renderUnits.push_back(renderUnit);
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

void AssetManager::TraverseSubMeshTree(SubMeshTree& subMeshTree, SubModel& subModel, VertexBuffer vb, IndexBuffer ib)
{
	static RenderUnitID largestIDinSubTree = 0;
	RenderUnitID lowestIDinSubTree = m_renderUnits.size() + 1;
	for (auto m : subMeshTree.subMeshes)
	{
		RenderUnit ru;
		MaterialProperties p = m.material.properties;
		MaterialProperties pbrP = m.pbrMaterial.properties;

		if (((pbrP & MaterialProperties::NORMAL_MAP) != 0) &&
			((pbrP & MaterialProperties::ALBEDO_MAP) != 0) &&
			((pbrP & MaterialProperties::METALLICROUGHNESS) != 0) &&
			((pbrP & MaterialProperties::IS_EMISSIVE) != 0))
		{
			PBR_ALBEDO_METROUG_NOR_EMIS mat;
			mat.albedoTextureID = this->LoadTex2D(m.pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
			mat.normalTextureID = this->LoadTex2D(m.pbrMaterial.normalPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
			mat.emissiveTextureID = this->LoadTex2D(m.pbrMaterial.emissivePath, LoadTexFlag::GenerateMips);
			mat.matallicRoughnessTextureID = this->LoadTex2D(m.pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

			mat.emissiveFactor = m.pbrMaterial.emissiveFactor;
			mat.rgba = m.pbrMaterial.baseColorFactor;
			mat.metallic = m.pbrMaterial.metallicFactor;
			mat.roughness = m.pbrMaterial.roughnessFactor;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PBR_ALBEDO_METROUG_NOR_EMIS;
		}
		else if (((pbrP & MaterialProperties::NORMAL_MAP) != 0) &&
			((pbrP & MaterialProperties::ALBEDO_MAP) != 0) &&
			((pbrP & MaterialProperties::METALLICROUGHNESS) != 0))
		{
			PBR_ALBEDO_METROUG_NOR mat;
			mat.albedoTextureID = this->LoadTex2D(m.pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
			mat.normalTextureID = this->LoadTex2D(m.pbrMaterial.normalPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
			mat.matallicRoughnessTextureID = this->LoadTex2D(m.pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

			mat.emissiveFactor = m.pbrMaterial.emissiveFactor;
			mat.rgba = m.pbrMaterial.baseColorFactor;
			mat.metallic = m.pbrMaterial.metallicFactor;
			mat.roughness = m.pbrMaterial.roughnessFactor;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PBR_ALBEDO_METROUG_NOR;
		}
		else if (((pbrP & MaterialProperties::ALBEDO_MAP) != 0) &&
			((pbrP & MaterialProperties::METALLICROUGHNESS) != 0))
		{
			PBR_ALBEDO_METROUG mat;
			mat.albedoTextureID = this->LoadTex2D(m.pbrMaterial.baseColorPath, LoadTexFlag::GenerateMips);
			mat.matallicRoughnessTextureID = this->LoadTex2D(m.pbrMaterial.metallicRoughnessPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);

			mat.emissiveFactor = m.pbrMaterial.emissiveFactor;
			mat.rgba = m.pbrMaterial.baseColorFactor;
			mat.metallic = m.pbrMaterial.metallicFactor;
			mat.roughness = m.pbrMaterial.roughnessFactor;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PBR_ALBEDO_METROUG;
		}
		else if ((pbrP & MaterialProperties::PBR) != 0)
		{
			PBR_NO_TEXTURES mat;

			mat.emissiveFactor = m.pbrMaterial.emissiveFactor;
			mat.rgba = m.pbrMaterial.baseColorFactor;
			mat.metallic = m.pbrMaterial.metallicFactor;
			mat.roughness = m.pbrMaterial.roughnessFactor;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PBR_NO_TEXTURES;
		}
		else if (((p & MaterialProperties::DIFFUSE_MAP) != 0) &&
			((p & MaterialProperties::NORMAL_MAP) != 0) &&
			((p & MaterialProperties::SPECULAR_MAP) != 0) &&
			((p & MaterialProperties::SHININESS) != 0))
		{
			PhongMaterial_DiffTex_NormTex_SpecTex mat;
			mat.diffuseTextureID = this->LoadTex2D(m.material.diffuseMapPath, LoadTexFlag::GenerateMips);
			mat.normalTextureID = this->LoadTex2D(m.material.normalMapPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
			mat.specularTextureID = this->LoadTex2D(m.material.specularMapPath, LoadTexFlag::GenerateMips);
			mat.shininess = m.material.shininess;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PhongMaterial_DiffTex_NormTex_SpecTex;
		}
		else if (((p & MaterialProperties::DIFFUSE_MAP) != 0) &&
			((p & MaterialProperties::NORMAL_MAP) != 0) &&
			((p & MaterialProperties::SPECULAR_COLOR) != 0) &&
			((p & MaterialProperties::SHININESS) != 0))
		{
			PhongMaterial_DiffTex_NormTex mat;
			mat.diffuseTextureID = this->LoadTex2D(m.material.diffuseMapPath, LoadTexFlag::GenerateMips);
			mat.normalTextureID = this->LoadTex2D(m.material.normalMapPath, LoadTexFlag::GenerateMips | LoadTexFlag::LinearColorSpace);
			mat.specularColor = m.material.colorSpecular;
			mat.shininess = m.material.shininess;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PhongMaterial_DiffTex_NormTex;
		}
		else if (((p & MaterialProperties::DIFFUSE_MAP) != 0) &&
			((p & MaterialProperties::SPECULAR_COLOR) != 0) &&
			((p & MaterialProperties::SHININESS) != 0))
		{
			PhongMaterial_DiffTex mat;
			mat.diffuseTextureID = this->LoadTex2D(m.material.diffuseMapPath, LoadTexFlag::GenerateMips);
			mat.specularColor = m.material.colorSpecular;
			mat.shininess = m.material.shininess;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PhongMaterial_DiffTex;
		}
		else if (((p & MaterialProperties::DIFFUSE_COLOR) != 0) &&
			((p & MaterialProperties::SPECULAR_COLOR) != 0) &&
			((p & MaterialProperties::SHININESS) != 0))
		{
			PhongMaterial_Color mat;
			mat.ambientColor = m.material.colorDiffuse;
			mat.diffuseColor = m.material.colorDiffuse;
			mat.specularColor = m.material.colorSpecular;
			mat.shininess = m.material.shininess;

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PhongMaterial_Color;
		}

		//assert(ru.material.type != MaterialType::none); //some material is missing
		if (ru.material.type == MaterialType::none) //some material is missing
		{
			PhongMaterial_Color mat;
			mat.ambientColor = rfm::Vector3(1, 0, 0);
			mat.diffuseColor = rfm::Vector3(1, 0, 0);

			ru.material.materialVariant = mat;
			ru.material.type = MaterialType::PhongMaterial_Color;
		}

		if (m.pbrMaterial.blendMode == BlendMode::blend || 
			m.material.properties == MaterialProperties::ALPHA_BLENDING ||
			m.material.properties == MaterialProperties::ALPHA_BLENDING_CONSTANS_OPACITY)
		{
			ru.material.renderFlag |= RenderFlag::alphaBlend;
		}
		else if (m.pbrMaterial.blendMode == BlendMode::mask ||
			m.material.properties == MaterialProperties::ALPHA_TESTING)
		{
			ru.material.renderFlag |= RenderFlag::alphaToCov;
		}

		if (m.pbrMaterial.twoSided || m.material.properties == MaterialProperties::NO_BACKFACE_CULLING)
		{
			ru.material.renderFlag |= RenderFlag::noBackFaceCull;
		}

		ru.subMesh.ib = ib;
		ru.subMesh.vb = vb;
		ru.subMesh.baseVertexLocation = m.vertexStart;
		ru.subMesh.startIndexLocation = m.indexStart;
		ru.subMesh.indexCount = m.indexCount;
		RenderUnitID ID = AddRenderUnit(ru);
		largestIDinSubTree = ID + 1;
		subModel.renderUnitIDs.push_back(ID);
	}
	for (int i = 0; i < subMeshTree.nodes.size(); i++)
	{
		SubModel newSubModel;
		TraverseSubMeshTree(subMeshTree.nodes[i], newSubModel, vb, ib);
		subModel.subModels.push_back(newSubModel);
	}
	subModel.RenderUnitBegin = subModel.renderUnitIDs.empty() ? lowestIDinSubTree : subModel.renderUnitIDs.front();	// hope this works
	subModel.RenderUnitEnd = largestIDinSubTree;				// hope this works
}

GID AssetManager::LoadModel(const std::string& filePath)
{
	if (m_filePathMap.contains(filePath))
	{
		return m_filePathMap[filePath];
	}
	GID newID = GID::GenerateNew();
	m_filePathMap[filePath] = newID;

	AssimpLoader a;
	EngineMeshData engineMeshData = a.loadStaticModel(filePath);

	Model model;
	model.ib = LowLvlGfx::CreateIndexBuffer(engineMeshData.getIndicesData(), engineMeshData.getIndicesCount());
	model.vb = LowLvlGfx::CreateVertexBuffer(engineMeshData.getVertextBuffer(), engineMeshData.getVertexCount() * engineMeshData.getVertexSize(), engineMeshData.getVertexSize());


	TraverseSubMeshTree(engineMeshData.subsetsInfo, model, model.vb, model.ib);

	m_models[newID] = model;

	return newID;
}

Model& AssetManager::GetModel(GID modelID)
{
	assert(m_models.contains(modelID));
	return m_models[modelID];
}



GID AssetManager::LoadTex2D(const std::string& path, LoadTexFlag flags)
{
	bool srgb = (flags & LoadTexFlag::LinearColorSpace) == 0;
	bool generateMips = (flags & LoadTexFlag::GenerateMips) != 0;


	if (m_filePathMap.contains(path))
	{
		return m_filePathMap[path];
	}
	MyImageStruct im;
	GID newID = GID::GenerateNew();
	m_filePathMap[path] = newID;
	readImage(im, path);
	D3D11_TEXTURE2D_DESC desc;
	desc.CPUAccessFlags = 0;

	desc.Height = im.height;
	desc.Width = im.width;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	if (generateMips)
	{
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		desc.MipLevels = im.mipNumber;
	}

	else
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.MiscFlags = 0;
		desc.MipLevels = 0;
	}

	std::shared_ptr<Texture2D> myTexture;

	if (generateMips)
	{
		D3D11_SUBRESOURCE_DATA* subResMipArray = nullptr;
		GfxHelpers::SetSubResDataMips(im.imagePtr, subResMipArray, im.mipNumber, im.stride);
		myTexture = LowLvlGfx::CreateTexture2D(desc, subResMipArray);
		delete[] subResMipArray;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		LowLvlGfx::CreateSRV(myTexture, &srvDesc);
	}
	else
	{
		D3D11_SUBRESOURCE_DATA subRes;
		subRes.pSysMem = im.imagePtr;
		subRes.SysMemPitch = im.stride;
		subRes.SysMemSlicePitch = 0;
		myTexture = LowLvlGfx::CreateTexture2D(desc, &subRes);
		LowLvlGfx::CreateSRV(myTexture, nullptr);
	}
	m_textures[newID] = myTexture;
	return newID;
}
