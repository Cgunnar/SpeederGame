#pragma once
#include "RimfrostMath.hpp"
#include "GraphicsResources.h"
#include "TerreinTypes.h"


class TerrainMeshGenerator
{
public:
	
	static TerrainMesh CreateTerrainMeshFromBMP(const std::string& fileName, TerrainMeshDesc desc = TerrainMeshDesc());
	
	static TerrainMesh CreateTerrainMesh(const TerrainMap& terrainMap, TerrainMeshDesc desc = TerrainMeshDesc());
	static void AsyncCreateTerrainMesh(const TerrainMap& terrainMap, std::function<void(TerrainMesh)> callback, TerrainMeshDesc desc = TerrainMeshDesc());
	static TerrainMesh CreateTerrainMeshFromHeightMapMemory(const float* hightMap, int width, int height, TerrainMeshDesc desc = TerrainMeshDesc());
private:
	static void CalcNormal(Triangle& tri);
	
	static void AsyncCreateTerrainMeshInternal(const TerrainMap& terrainMap, std::function<void(TerrainMesh)> callback, TerrainMeshDesc desc);
	static std::unordered_map<rfm::Vector2I, TerrainMap> s_terrainMapHolder;
	static std::mutex s_mapMutex;
};

