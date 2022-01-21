#include "pch.hpp"
#include "TerrainTypes.h"

#include "TerrainMeshGenerator.h"
#include "AssetManager.h"

TerrainLODMesh::TerrainLODMesh(int lod) : m_lod(lod)
{
}

TerrainLODMesh::~TerrainLODMesh()
{
	assert(!this->waitingOnMesh);
	this->Reset();
}

void TerrainLODMesh::OnReceive(TerrainMesh&& mesh)
{
	std::lock_guard g(m_mutex);
	if (waitingOnMesh)
	{
		this->mesh = std::move(mesh);
		waitingOnMesh = false;
		hasMesh = true;
		hasTriangles = true;
	}
}



void TerrainLODMesh::RequestMesh(const TerrainMap& map, TerrainMeshDesc desc)
{
	std::lock_guard g(m_mutex);
	assert(!waitingOnMesh);
	waitingOnMesh = true;
	desc.LOD = this->m_lod;

	TerrainMeshGenerator::AsyncCreateTerrainMesh(map, [&](TerrainMesh&& mesh) {
		OnReceive(std::move(mesh));
		}, desc);
}

void TerrainLODMesh::GenerateRenderMesh(MeshFormat format)
{
	std::lock_guard g(m_mutex);
	meshFormat = format;
	assert(!hasRenderMesh);
	if (meshFormat == MeshFormat::POS_NOR_UV_TAN_BITAN)
	{
		renderMesh = AssetManager::Get().AddMesh(Mesh(mesh.verticesTBN, mesh.indices));
	}
	else
	{
		renderMesh = AssetManager::Get().AddMesh(Mesh(mesh.vertices, mesh.indices));
	}
	this->mesh.indices.clear();
	this->mesh.vertices.clear();
	this->mesh.verticesTBN.clear();
	if (m_lod != 0)
	{
		this->mesh.triangles.clear();
		hasTriangles = false;
	}
	hasRenderMesh = true;
}

void TerrainLODMesh::Reset()
{
	std::lock_guard g(m_mutex);
	this->mesh.indices.clear();
	this->mesh.vertices.clear();
	this->mesh.verticesTBN.clear();
	this->mesh.triangles.clear();
	this->waitingOnMesh = false;
	this->hasTriangles = false;
	this->hasMesh = false;
	if (hasRenderMesh)
	{
		AssetManager::Get().RemoveMesh(renderMesh);
	}
	this->hasRenderMesh = false;
	
}

int TerrainLODMesh::GetLod() const
{
	return m_lod;
}
