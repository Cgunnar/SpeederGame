#pragma once

#include "RimfrostMath.hpp"
#include "TerreinTypes.h"


class TerrainMapGenerator
{
public:
	static constexpr int chunkSize = 241;
	
	static TerrainMap GenerateTerrinMap(const TerrainMapDesc& mapDesc);
	static void AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord);
	static std::optional<TerrainMap> GetTerrainMap(rfm::Vector2I coord);
	static void Destroy();

private:
	static std::vector<float> GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed);

	static void AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord);

	static std::unordered_map<rfm::Vector2I, TerrainMap> s_terrainMapHolder;
	static std::mutex s_mapMutex;
};

