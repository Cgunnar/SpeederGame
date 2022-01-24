#pragma once

#include "RimfrostMath.hpp"
#include "TerrainTypes.h"



class TerrainMapGenerator
{
public:
	static void Init();
	static bool IsInitialized();
	static TerrainMap GenerateTerrinMap(const TerrainMapDesc& mapDesc);
	static void AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord);
	static std::optional<TerrainMap> GetTerrainMap(rfm::Vector2I coord);
	static void RemoveTerrainMap(rfm::Vector2I coord);
	static void Destroy();

private:
	static std::vector<float> GenerateNoise(int width, int height, const TerrainMapDesc& mapDesc);

	static void BlendEdge(TerrainMap& centerMap, TerrainMap* leftMap, TerrainMap* rightMap, TerrainMap* upMap, TerrainMap* downMap,
		TerrainMap* leftUpMap, TerrainMap* righUptMap, TerrainMap* leftDownMap, TerrainMap* rightDownMap);
	static void AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord, uint64_t frameNumber);
	static bool s_initialized;
	struct TerrainMapAndMetaData
	{
		TerrainMap map;
		TerrainMapDesc desc;
		uint64_t frameRequested;
	};
	static std::unordered_map<rfm::Vector2I, TerrainMapAndMetaData> s_terrainMapHolder;
	static std::mutex s_mapMutex;
};

