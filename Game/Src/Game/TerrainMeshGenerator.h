#pragma once
#include "RimfrostMath.hpp"
#include "GraphicsResources.h"

class TerrainMeshGenerator
{
public:
	TerrainMeshGenerator() = default;
	~TerrainMeshGenerator();
	
	void CreateTerrainFromBMP(const std::string& fileName, float scale = 10, rfm::Vector2 uvScale = { 1,1 });

	//bool GetHeight(float& heightOut) const;
	const std::vector<Vertex_POS_NOR_UV>& GetVertices() const;
	const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& GetVerticesTBN() const;
	const std::vector<uint32_t>& GetIndices() const;
	static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	static constexpr uint32_t vertexStrideTBN = sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
	void CreateTerrain(float scale, const float* hightMap, int width, int height, rfm::Vector2 uvScale = 1);
private:
	void CalcNormal(Triangle& tri) const;
	std::vector<Triangle> m_triangles;
	std::vector<Vertex_POS_NOR_UV> m_vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> m_verticesTBN;
	std::vector<uint32_t> m_indices;
};

