#pragma once

#include "RimfrostMath.hpp"
#include "TerrainMeshGenerator.h"


class TerrainMapGenerator
{
public:
	static constexpr int chunkSize = 241;
	
	static TerrainMap GenerateTerrinMap(const TerrainMapDesc& mapDesc);
	static void AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord);
	static std::optional<TerrainMap> GetTerrainMap(rfm::Vector2I coord);
	static void Init();
	static void Destroy();

private:
	static std::vector<float> GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed);

	static void AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord);
	static void JoinThreads();

	static std::unordered_map<rfm::Vector2I, TerrainMap> s_terrainMapHolder;
	static std::mutex s_mapMutex;
	static std::mutex s_threadQueueMutex;
	static std::queue<std::thread>* s_threadQueue;
	static std::thread* s_joinerthread;
	static std::atomic<bool> s_killThreadJoiner;
};

