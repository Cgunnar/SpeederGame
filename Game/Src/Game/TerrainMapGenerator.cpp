#include "pch.hpp"
#include "TerrainMapGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "UtilityFunctions.h"
#include "WorkerThreads.h"

using namespace rfm;

std::mutex TerrainMapGenerator::s_mapMutex;

std::unordered_map<rfm::Vector2I, TerrainMap> TerrainMapGenerator::s_terrainMapHolder;


void TerrainMapGenerator::AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord)
{
	//TerrainMap&& newMap = TerrainMapGenerator::GenerateTerrinMap(mapDesc);
	TerrainMap newMap = TerrainMapGenerator::GenerateTerrinMap(mapDesc);

	assert(!s_terrainMapHolder.contains(coord));
	s_mapMutex.lock();
	//s_terrainMapHolder[coord] = std::move(newMap);
	s_terrainMapHolder[coord] = newMap;
	s_mapMutex.unlock();
}


TerrainMap TerrainMapGenerator::GenerateTerrinMap(const TerrainMapDesc& mapDesc)
{

	TerrainMap map;
	map.height = chunkSize;
	map.width = chunkSize;
	map.heightMap = GenerateNoise(chunkSize, chunkSize, mapDesc.scale, mapDesc.octaves,
		mapDesc.persistence, mapDesc.lacunarity, mapDesc.offset, mapDesc.seed);


	std::vector<Vector4> colorMap;
	colorMap.resize(chunkSize * (size_t)chunkSize);
	for (int i = 0; i < chunkSize * chunkSize; i++)
	{
		for (auto& b : mapDesc.bioms)
		{
			if (map.heightMap[i] <= b.threshold)
			{
				if(b.flat) map.heightMap[i] = b.threshold;
				colorMap[i] = Vector4(b.color, 1);
				break;
			}
		}
	}

	map.colorMapRGBA = util::FloatToCharRGBA((float*)colorMap.data(), chunkSize, chunkSize);
	return map;
}

void TerrainMapGenerator::AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord)
{
	assert(!s_terrainMapHolder.contains(coord));
	
	//s_terrainMapHolder[coord] = TerrainMapGenerator::GenerateTerrinMap(mapDesc);
	WorkerThreads::AddTask(TerrainMapGenerator::AsyncGenerateTerrinMapInternal, mapDesc, coord);
	return;
}

std::optional<TerrainMap> TerrainMapGenerator::GetTerrainMap(rfm::Vector2I coord)
{
	std::optional<TerrainMap> outOpt = std::nullopt;
	s_mapMutex.lock();
	if (s_terrainMapHolder.contains(coord))
	{
		outOpt = std::make_optional(std::move(s_terrainMapHolder[coord]));
		s_terrainMapHolder.erase(coord);
	}
	s_mapMutex.unlock();
	return outOpt;
}

void TerrainMapGenerator::Destroy()
{
	s_terrainMapHolder.clear();
}


std::vector<float> TerrainMapGenerator::GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
	rfm::Vector2 offset, uint32_t seed)
{
	assert(width > 0 && height > 0 && scale > 0 && octaves >= 1 && lacunarity >= 1 && 0 <= persistance && persistance <= 1);

	Vector2* octRandOffsets = new Vector2[octaves]();
	for (int i = 0; i < octaves; i++)
	{
		octRandOffsets[i].x = GenRandFloat(-10000, 10000, seed + octaves) + offset.x;
		octRandOffsets[i].y = GenRandFloat(-10000, 10000, (int)octRandOffsets[i].x) + offset.y;
	}

	const siv::PerlinNoise::seed_type sivseed = seed;
	const siv::PerlinNoise perlin{ sivseed };

	std::vector<float> noise;
	noise.resize((size_t)width * (size_t)height);

	float maxNoise = std::numeric_limits<float>::min();
	float minNoise = std::numeric_limits<float>::max();
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float amplitude = 1;
			float frequency = 1;	
			float noiseHeight = 0;

			for (int i = 0; i < octaves; i++)
			{
				float sampleX = ((float)x - width / 2.0f)  * frequency / scale + octRandOffsets[i].x;
				float sampleY = ((float)y - height / 2.0f) * frequency / scale + octRandOffsets[i].y;

				float perlinNoise = amplitude * static_cast<float>(perlin.noise2D(sampleX, sampleY));
				noiseHeight += perlinNoise;
				amplitude *= persistance;
				frequency *= lacunarity;
			}
			if (noiseHeight > maxNoise)
			{
				maxNoise = noiseHeight;
			}
			else if (noiseHeight < minNoise)
			{
				minNoise = noiseHeight;
			}

			noise[y * static_cast<size_t>(width) + x] = noiseHeight;
		}
	}
	for (int i = 0; i < height * width; i++)
	{
 		noise[i] = rfm::InvLerp(minNoise, maxNoise, noise[i]);
	}
	

	
	delete[] octRandOffsets;
	return noise;
}
