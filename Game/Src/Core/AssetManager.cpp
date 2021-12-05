#include "pch.hpp"
#include "AssetManager.h"
#include "Geometry.h"
#include "LowLvlGfx.h"

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
	m_meshes.push_back(Mesh());
	Mesh& mesh = m_meshes.back();
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

const Mesh& AssetManager::GetMesh(MeshID id) const
{
	assert(id > 0 && id-1 < m_meshes.size());
	return m_meshes[id - 1];
}

const Mesh& AssetManager::GetMesh(SimpleMesh mesh) const
{
	MeshID id = static_cast<MeshID>(mesh);
	assert(id > 0 && id - 1 < m_meshes.size());
	return m_meshes[id - 1];
}


GID AssetManager::AddTexture2D(std::shared_ptr<Texture2D> tempArgumentFixCreationOfTexture2dLater)
{
	GID id = GID::GenerateNew();
	m_textures[id] = tempArgumentFixCreationOfTexture2dLater;
	return id;
}

MeshID AssetManager::AddMesh(Mesh mesh)
{
	m_meshes.push_back(mesh);
	return m_meshes.size(); // MeshID will always be index + 1
}


