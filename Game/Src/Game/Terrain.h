#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class TerrainScript;
class TerrainChunk
{
	friend TerrainScript;
public:
	TerrainChunk() = default;
	TerrainChunk(rfm::Vector2I coord, int size);
	void Update(rfm::Vector2 viewPos, float maxViewDist);
private:
	rfm::Vector2 m_position;
	rfm::Vector3 m_topLeft;
	rfm::Vector3 m_topRight;
	rfm::Vector3 m_botLeft;
	rfm::Vector3 m_botRight;
	bool m_visible = false;
	rfe::Entity m_terrainMesh;
};

