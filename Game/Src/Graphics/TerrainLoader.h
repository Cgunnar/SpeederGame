#pragma once
#include "RimfrostMath.hpp"
#include "GraphicsResources.h"

class TerrainLoader
{
public:
	TerrainLoader() = default;
	~TerrainLoader();
	
	void CreateTerrainFromBMP(const std::string& fileName, float scale = 0.05f);

	//bool GetHeight(float& heightOut) const;
	const std::vector<Vertex_POS_NOR_UV>& GetVertices() const;
	const std::vector<uint32_t>& GetIndices() const;
	static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
private:
	void CreateTerrain();
	void CalcNormal(Triangle& tri) const;
	std::vector<Triangle> m_triangles;
	std::vector<Vertex_POS_NOR_UV> m_vertices;
	std::vector<uint32_t> m_indices;
	uint8_t* m_heightMapData = nullptr;
	struct HeightMap
	{
		size_t width = 0;
		size_t height = 0;
		Vertex_POS_NOR_UV* vertices = nullptr;
	} m_heightMap{};

};

