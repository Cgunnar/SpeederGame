#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
#include "TerreinTypes.h"

class TerrainLODMesh
{
public:
	TerrainLODMesh(int lod);
	TerrainLODMesh() = default;
	void OnReceive(TerrainMesh mesh);
	void RequestMesh(TerrainMap map);
private:
	TerrainMesh m_mesh;
	bool m_hasRequestedMesh = false;
	bool m_hasMesh = false;
	int m_lod;
};

class TerrainScript;
class TerrainChunk
{
	friend TerrainScript;
public:
	TerrainChunk() = default;
	TerrainChunk(rfm::Vector2I coord, int size);
	void Update(rfm::Vector2 viewPos, float maxViewDist);
	void LoadTerrain(const TerrainMapDesc& desc);
private:
	void OnReceive(TerrainMesh&& mesh);
	rfm::Vector2 m_position;
	rfm::Vector2I m_coord;
	rfm::Vector3 m_topLeft;
	rfm::Vector3 m_topRight;
	rfm::Vector3 m_botLeft;
	rfm::Vector3 m_botRight;
	bool m_visible = false;
	bool m_checkForLoadedTerrainMap = false;
	bool m_createRenderMesh = false;
	TerrainMesh m_mesh;
	rfe::Entity m_chunkEntity;
};

