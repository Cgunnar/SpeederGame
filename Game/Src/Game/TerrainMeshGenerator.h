#pragma once
#include "RimfrostMath.hpp"
#include "GraphicsResources.h"
#include "TerreinTypes.h"

class TerrainMeshGenerator
{
public:
	TerrainMeshGenerator() = default;
	~TerrainMeshGenerator();
	
	void CreateTerrainMeshFromBMP(const std::string& fileName, float scale, int LOD, rfm::Vector2 uvScale = { 1,1 });

	//bool GetHeight(float& heightOut) const;
	const std::vector<Vertex_POS_NOR_UV>& GetVertices() const;
	const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& GetVerticesTBN() const;
	const std::vector<uint32_t>& GetIndices() const;
	static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	static constexpr uint32_t vertexStrideTBN = sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
	void CreateTerrainMesh(TerrainMap terrainMap, float scale, int LOD = 1, rfm::Vector2 uvScale = 1, std::function<float(float)> heightScaleFunc = [](float s) {return s; });
	void CreateTerrainMeshFromHeightMapMemory(const float* hightMap, int width, int height, float scale, int LOD,
		rfm::Vector2 uvScale = 1, std::function<float(float)> heightScaleFunc = [](float s) {return s; });
private:
	void CalcNormal(Triangle& tri) const;
	std::vector<Triangle> m_triangles;
	std::vector<Vertex_POS_NOR_UV> m_vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> m_verticesTBN;
	std::vector<uint32_t> m_indices;
};

