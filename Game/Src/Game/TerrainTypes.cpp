#include "pch.hpp"
#include "TerreinTypes.h"

#include "TerrainMeshGenerator.h"
#include "AssetManager.h"

TerrainLODMesh::TerrainLODMesh(int lod) : m_lod(lod)
{
}

void TerrainLODMesh::OnReceive(TerrainMesh&& mesh)
{
	this->mesh = std::move(mesh);
	hasMesh = true;
}

void TerrainLODMesh::RequestMesh(const TerrainMap& map, TerrainMeshDesc desc)
{
	hasRequestedMesh = true;
	desc.LOD = this->m_lod;

	TerrainMeshGenerator::AsyncCreateTerrainMesh(map, [&](TerrainMesh&& mesh) {
		OnReceive(std::move(mesh));
		}, desc);
}

void TerrainLODMesh::GenerateRenderMesh(MeshFormat format)
{
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
	this->mesh = TerrainMesh();
	hasRenderMesh = true;
}
