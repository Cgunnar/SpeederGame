#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
#include "TerreinTypes.h"



class TerrainScript;
class TerrainChunk
{
	friend TerrainScript;
public:
	TerrainChunk() = default;
	TerrainChunk(rfm::Vector2I coord, int size, rfm::Transform terrainTransform, TerrainMeshDesc mapDesc, std::vector<LODinfo> lods);
	void Update(rfm::Vector2 viewPos, float maxViewDist);
	void LoadTerrain(const TerrainMapDesc& desc);
	void UpdateChunkTransform(rfm::Transform transform);
private:
	rfe::Entity m_chunkEntity;
	rfm::Vector2I m_coord;
	rfm::Vector2 m_position;
	rfm::Vector3 m_corners[4];
	TerrainMeshDesc m_meshDesc;
	TerrainMap m_map;
	Material m_material;
	std::vector<LODinfo> m_lods;
	std::vector<TerrainLODMesh> m_lodMeshes;
	bool m_visible = false;
	bool m_checkForLoadedTerrainMap = false;
	bool m_createRenderMesh = false;
	int m_prevLODindex = -1;
	bool m_hasMap = false;
};

