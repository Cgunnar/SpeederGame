#pragma once
#include "RimfrostMath.hpp"
#include "GraphicsResources.h"
#include "TerreinTypes.h"

struct TerrainMesh
{
	static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	static constexpr uint32_t vertexStrideTBN = sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
	std::vector<Triangle> triangles;
	std::vector<Vertex_POS_NOR_UV> vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> verticesTBN;
	std::vector<uint32_t> indices;
};
class TerrainMeshGenerator
{
public:
	TerrainMeshGenerator() = default;
	~TerrainMeshGenerator();
	
	TerrainMesh CreateTerrainMeshFromBMP(const std::string& fileName, float scale, int LOD, rfm::Vector2 uvScale = { 1,1 });
	
	TerrainMesh CreateTerrainMesh(TerrainMap terrainMap, float scale, int LOD = 1, rfm::Vector2 uvScale = 1, std::function<float(float)> heightScaleFunc = [](float s) {return s; });
	TerrainMesh CreateTerrainMeshFromHeightMapMemory(const float* hightMap, int width, int height, float scale, int LOD,
		rfm::Vector2 uvScale = 1, std::function<float(float)> heightScaleFunc = [](float s) {return s; });
private:
	void CalcNormal(Triangle& tri) const;
	

	static std::unordered_map<rfm::Vector2I, TerrainMap> s_terrainMapHolder;
	static std::mutex s_mapMutex;
};

