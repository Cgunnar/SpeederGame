#include "pch.hpp"
#include "TerreinTypes.h"

#include "TerrainMeshGenerator.h"

TerrainLODMesh::TerrainLODMesh(int lod) : m_lod(lod)
{

}

void TerrainLODMesh::OnReceive(TerrainMesh&& mesh)
{
	this->mesh = std::move(mesh);
	hasMesh = true;
}

void TerrainLODMesh::RequestMesh(const TerrainMap& map)
{
	hasRequestedMesh = true;
	TerrainMeshDesc meshDesc;
	meshDesc.heightScaleFunc = [](float in) {return in <= 0.3f ? 0.3f * 0.3f : in * in; };
	meshDesc.LOD = this->m_lod;

	TerrainMeshGenerator::AsyncCreateTerrainMesh(map, [&](TerrainMesh&& mesh) {
		OnReceive(std::move(mesh));
		}, meshDesc);
}

void TerrainLODMesh::GenerateRenderMesh(MeshFormat format)
{
	meshFormat = format;
	assert(!hasRenderMesh);
	if (meshFormat == MeshFormat::POS_NOR_UV_TAN_BITAN)
	{
		renderMesh = SubMesh(mesh.verticesTBN, mesh.indices);
	}
	else
	{
		renderMesh = SubMesh(mesh.vertices, mesh.indices);
	}
	this->mesh = TerrainMesh();
	hasRenderMesh = true;
}
