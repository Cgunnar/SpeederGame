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

	//-----------------------

	Geometry::Sphere_POS_NOR_UV_TAN_BITAN sphere(16);
	m_renderUnits.push_back(RenderUnit());
	SubMesh& sphereMesh = m_renderUnits.back().subMesh;
	sphereMesh.ib = LowLvlGfx::CreateIndexBuffer(sphere.IndexData(), sphere.IndexCount());
	sphereMesh.vb = LowLvlGfx::CreateVertexBuffer(sphere.VertexData(), sphere.ArraySize(), sphere.vertexStride);
	sphereMesh.baseVertexLocation = 0;
	sphereMesh.startIndexLocation = 0;
	sphereMesh.indexCount = sphereMesh.ib.GetIndexCount();

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

const SubMesh& AssetManager::GetMesh(GID id) const
{
	assert(m_meshes.contains(id));
	return m_meshes.at(id);
}

const RenderUnit& AssetManager::GetRenderUnit(RenderUnitID id) const
{
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return m_renderUnits[id - 1];
}

RenderUnit& AssetManager::GetRenderUnit(RenderUnitID id)
{
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return m_renderUnits[id - 1];
}


RenderUnitID AssetManager::AddMesh(SubMesh mesh)
{
	m_renderUnits.push_back({ mesh, MaterialVariant() });
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(RenderUnit renderUnit)
{
	m_renderUnits.push_back(renderUnit);
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(const SubMesh& subMesh, const Material& material)
{
	m_renderUnits.push_back({ subMesh, MaterialVariant(material)});
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

GID AssetManager::LoadMesh(const std::string& path, MeshFormat format)
{
	if (m_meshFilePathMap.contains(path))
	{
		return m_meshFilePathMap[path];
	}
	SubMesh mesh;

	m_meshFilePathMap[path] = mesh.GetGID();

	AssimpLoader a;
	EngineMeshData engineMeshData = a.loadStaticModel(path);
	
	mesh.ib = LowLvlGfx::CreateIndexBuffer(engineMeshData.getIndicesData(), (uint32_t)engineMeshData.getIndicesCount());
	mesh.vb = LowLvlGfx::CreateVertexBuffer(engineMeshData.getVertextBuffer(format),
		engineMeshData.getVertexCount(format) * engineMeshData.getVertexSize(format),
		(uint32_t)engineMeshData.getVertexSize(format));

	mesh.baseVertexLocation = 0;
	mesh.startIndexLocation = 0;
	mesh.indexCount = mesh.ib.GetIndexCount();
	
	assert(!m_meshes.contains(mesh.GetGID()));
	m_meshes[mesh.GetGID()] = mesh;

	return mesh.GetGID();
}

void AssetManager::TraverseSubMeshTree(SubMeshTree& subMeshTree, SubModel& subModel, VertexBuffer vb, IndexBuffer ib)
{
	static RenderUnitID largestIDinSubTree = 0;
	RenderUnitID lowestIDinSubTree = m_renderUnits.size() + 1;
	for (auto m : subMeshTree.subMeshes)
	{
		RenderUnit ru;
		MaterialProperties pbrP = m.pbrMaterial.properties;

		ru.material = MaterialVariant(m.pbrMaterial);

		//assert(ru.material.type != MaterialType::none); //some material is missing
		if (ru.material.type == MaterialVariantEnum::none) //some material is missing
		{
			Material mat;
			mat.emissiveFactor = rfm::Vector3(1, 0, 0);
			mat.baseColorFactor = rfm::Vector4(1, 0, 0, 1);

			ru.material.materialVariant = mat;
			ru.material.type = MaterialVariantEnum::PBR_NO_TEXTURES;
		}

		if (m.pbrMaterial.blendMode == BlendMode::blend)
		{
			ru.material.renderFlag |= RenderFlag::alphaBlend;
		}
		else if (m.pbrMaterial.blendMode == BlendMode::mask)
		{
			ru.material.renderFlag |= RenderFlag::alphaToCov;
		}

		if ((m.pbrMaterial.properties & MaterialProperties::NO_BACKFACE_CULLING) != 0)
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
	model.ib = LowLvlGfx::CreateIndexBuffer(engineMeshData.getIndicesData(), (uint32_t)engineMeshData.getIndicesCount());
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



GID AssetManager::LoadTex2DFromFile(const std::string& path, LoadTexFlag flags)
{
	bool srgb = (flags & LoadTexFlag::LinearColorSpace) == 0;
	bool generateMips = (flags & LoadTexFlag::GenerateMips) != 0;


	if (m_filePathMap.contains(path))
	{
		return m_filePathMap[path];
	}
	MyImageStruct im;
	readImage(im, path);

	GID newID = LoadTex2DFromMemoryR8G8B8A8((unsigned char*)im.imagePtr, im.width, im.height, flags);
	m_filePathMap[path] = newID;
	return newID;
}

GID AssetManager::LoadTex2DFromMemoryR8G8B8A8(const unsigned char* src, int width, int height, LoadTexFlag flags)
{
	assert(width > 0 && height > 0 && src);
	bool srgb = (flags & LoadTexFlag::LinearColorSpace) == 0;
	bool generateMips = (flags & LoadTexFlag::GenerateMips) != 0;

	GID newID = GID::GenerateNew();

	D3D11_TEXTURE2D_DESC desc;
	desc.CPUAccessFlags = 0;

	desc.Height = height;
	desc.Width = width;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	if (generateMips)
	{
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		desc.MipLevels = GfxHelpers::CalcMipNumber(width, height);
	}

	else
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.MiscFlags = 0;
		desc.MipLevels = 1;
	}

	std::shared_ptr<Texture2D> myTexture;

	if (generateMips)
	{
		D3D11_SUBRESOURCE_DATA* subResMipArray = nullptr;
		GfxHelpers::SetSubResDataMips(src, subResMipArray, desc.MipLevels, width * 4);
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
		subRes.pSysMem = src;
		subRes.SysMemPitch = width * 4;
		subRes.SysMemSlicePitch = 0;
		myTexture = LowLvlGfx::CreateTexture2D(desc, &subRes);
		LowLvlGfx::CreateSRV(myTexture, nullptr);
	}
	m_textures[newID] = myTexture;
	return newID;
}
