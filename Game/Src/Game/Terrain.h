#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class TerrainChunk
{
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

	rfe::Entity m_terrainMesh;
};

