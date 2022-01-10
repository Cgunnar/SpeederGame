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
	Mesh mesh = Mesh(quad2.VertexData(), quad2.IndexData());
	mesh.guid = static_cast<uint64_t>(SimpleMesh::Quad_POS_NOR_UV);
	AddMesh(mesh);

	//-----------------------

	Geometry::Sphere_POS_NOR_UV_TAN_BITAN sphere(16);
	mesh = Mesh(sphere.VertexData(), sphere.IndexData());
	mesh.guid = static_cast<uint64_t>(SimpleMesh::UVSphere_POS_NOR_UV_TAN_BITAN);
	AddMesh(mesh);

	//----------------------
	Geometry::AABB_POS_NOR_UV aabb(AABB({ -0.5f,-0.5f,-0.5f }, { 0.5f,0.5f,0.5f }));
	mesh = Mesh(aabb.VertexData(), aabb.IndexData());
	mesh.guid = static_cast<uint64_t>(SimpleMesh::BOX_POS_NOR_UV);
	AddMesh(mesh);
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

void AssetManager::RemoveTexture2D(GID guid)
{
	if (m_textures.contains(guid))
	{
		m_textures.erase(guid);
	}
}

Mesh AssetManager::GetMesh(RenderUnitID id) const
{
	assert(id > 0 && id - 1 < m_renderUnits.size());
	return GetMesh(m_renderUnits[id - 1].meshID);
}

Mesh AssetManager::GetMesh(SimpleMesh mesh) const
{
	assert(m_meshes.contains(GID(mesh)));
	return m_meshes.at(GID(mesh));
}

Mesh AssetManager::GetMesh(GID id) const
{
	assert(m_meshes.contains(id));
	return m_meshes.at(id);
}

void AssetManager::RemoveMesh(GID id)
{
	if (id == 0) return;
	assert(m_meshes.contains(id));
	m_meshes.erase(id);
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


GID AssetManager::AddMesh(Mesh mesh)
{
	//m_renderUnits.push_back({ mesh, Material() });
	//return m_renderUnits.size(); // RenderUnitID will always be index + 1
	assert(!m_meshes.contains(mesh.GetGID()));
	m_meshes[mesh.GetGID()] = mesh;
	return mesh.GetGID();
}

RenderUnitID AssetManager::AddRenderUnit(RenderUnit renderUnit)
{
	m_renderUnits.push_back(renderUnit);
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(const Mesh& mesh, const Material& material)
{
	GID meshID = AddMesh(mesh);
	m_renderUnits.push_back({ meshID, material});
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

RenderUnitID AssetManager::AddRenderUnit(GID meshID, const Material& material)
{
	m_renderUnits.push_back({ meshID, material });
	return m_renderUnits.size(); // RenderUnitID will always be index + 1
}

GID AssetManager::LoadMesh(const std::string& path, MeshFormat format)
{
	if (m_meshFilePathMap.contains(path))
	{
		return m_meshFilePathMap[path];
	}
	Mesh mesh;

	m_meshFilePathMap[path] = mesh.GetGID();

	AssimpLoader a;
	EngineMeshData engineMeshData = a.loadStaticModel(path);
	Mesh m = Mesh(LowLvlGfx::CreateVertexBuffer(engineMeshData.getVertextBuffer(format),
		engineMeshData.getVertexCount(format) * engineMeshData.getVertexSize(format),
		(uint32_t)engineMeshData.getVertexSize(format)),
		LowLvlGfx::CreateIndexBuffer(engineMeshData.getIndicesData(),
			(uint32_t)engineMeshData.getIndicesCount()), { {0,0,0},{0,0,0} });
	std::cout << "fix aabb for AssetManager::LoadMesh" << std::endl;

	assert(!m_meshes.contains(mesh.GetGID()));
	m_meshes[mesh.GetGID()] = mesh;

	return mesh.GetGID();
}

void AssetManager::TraverseSubMeshTree(SubMeshTree& subMeshTree, SubModel& subModel, VertexBuffer vb, IndexBuffer ib)
{
	static RenderUnitID largestIDinSubTree = 0;
	RenderUnitID lowestIDinSubTree = m_renderUnits.size() + 1;
	AABB aabb;
	for (auto m : subMeshTree.subMeshes)
	{
		aabb = AABB::Merge(aabb, m.aabb);
		RenderUnit ru;
		ru.material = m.pbrMaterial;
		ru.meshID = AssetManager::Get().AddMesh(Mesh(vb, ib, m.indexCount, m.indexStart, m.vertexStart, m.aabb));
		RenderUnitID ID = AddRenderUnit(ru);
		largestIDinSubTree = ID + 1;
		subModel.renderUnitIDs.push_back(ID);
	}
	for (int i = 0; i < subMeshTree.nodes.size(); i++)
	{
		SubModel newSubModel;
		TraverseSubMeshTree(subMeshTree.nodes[i], newSubModel, vb, ib);
		aabb = AABB::Merge(aabb, newSubModel.aabb);
		subModel.subModels.push_back(newSubModel);
	}
	subModel.aabb = aabb;
	subModel.RenderUnitBegin = subModel.renderUnitIDs.empty() ? lowestIDinSubTree : subModel.renderUnitIDs.front();	// hope this works
	subModel.RenderUnitEnd = largestIDinSubTree;
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
