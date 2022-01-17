#include "pch.hpp"
#include "TerrainMapGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "UtilityFunctions.h"
#include "WorkerThreads.h"
#include "Hydraulic_Erosion.h"
#include "Timer.hpp"

using namespace rfm;

std::mutex TerrainMapGenerator::s_mapMutex;

std::unordered_map<rfm::Vector2I, TerrainMap> TerrainMapGenerator::s_terrainMapHolder;
bool TerrainMapGenerator::s_initialized = false;





void TerrainMapGenerator::AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord)
{
	TerrainMap&& newMap = TerrainMapGenerator::GenerateTerrinMap(mapDesc);
	s_mapMutex.lock();
	assert(!s_terrainMapHolder.contains(coord));
	s_terrainMapHolder[coord] = std::move(newMap);
	s_mapMutex.unlock();
}


void TerrainMapGenerator::Init()
{
	ErosionSimulator::InitializeBrush(chunkSize, 3);
	s_initialized = true;
}

void TerrainMapGenerator::Destroy()
{
	s_terrainMapHolder.clear();
}

bool TerrainMapGenerator::IsInitialized()
{
	return s_initialized;
}

TerrainMap TerrainMapGenerator::GenerateTerrinMap(const TerrainMapDesc& mapDesc)
{

	TerrainMap map;
	map.height = chunkSize;
	map.width = chunkSize;
	map.heightMap = GenerateNoise(chunkSize, chunkSize, mapDesc.frequencyScale, mapDesc.octaves,
		mapDesc.persistence, mapDesc.lacunarity, mapDesc.offset, mapDesc.seed);
	Timer timer;
	ErosionSimulator::Erode(map);
	auto t = timer.stop();
	std::cout << t << " " << timer.durationToString() << std::endl;

	std::vector<Vector4> colorMap;
	colorMap.resize(chunkSize * (size_t)chunkSize);
	for (int i = 0; i < chunkSize * chunkSize; i++)
	{
		for (int j = 0; j < mapDesc.bioms.size(); j++)
		{
			if (map.heightMap[i] >= mapDesc.bioms[j].threshold && j < mapDesc.bioms.size() - 1)
			{
				if (map.heightMap[i] < mapDesc.bioms[(size_t)j + 1].threshold)
				{
					if(mapDesc.bioms[j].flat) map.heightMap[i] = 0.99f * mapDesc.bioms[j+1].threshold;
					colorMap[i] = Vector4(mapDesc.bioms[j].color, 1);
				}
			}
			else if (map.heightMap[i] >= mapDesc.bioms[j].threshold)
			{
				if (mapDesc.bioms[j].flat) map.heightMap[i] = mapDesc.bioms[j].threshold;
				colorMap[i] = Vector4(mapDesc.bioms[j].color, 1);
			}
			else
			{
				break;
			}
		}
	}

	map.colorMapRGBA = util::FloatToCharRGBA((float*)colorMap.data(), chunkSize, chunkSize);
	return map;
}

void TerrainMapGenerator::AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord)
{
	if (s_terrainMapHolder.contains(coord))
	{
		//s_terrainMapHolder.erase(coord);
		return;
	}
	
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
		//outOpt = std::make_optional(std::move(s_terrainMapHolder[coord]));
		//s_terrainMapHolder.erase(coord);
		auto& map = s_terrainMapHolder[coord];
		if (map.blendedEdges)
		{
			outOpt = std::make_optional(s_terrainMapHolder[coord]);
			std::cout << "TerrainMapGenerator::GetTerrainMap fix så that terrainMaps are deleted when unloaded" << std::endl;
		}
		else if (s_terrainMapHolder.contains(coord + Vector2I(0, 1)) && s_terrainMapHolder.contains(coord + Vector2I(0, -1)) &&
			s_terrainMapHolder.contains(coord + Vector2I(1, 0)) && s_terrainMapHolder.contains(coord + Vector2I(-1, 0)) &&
			s_terrainMapHolder.contains(coord + Vector2I(-1, 1)) && s_terrainMapHolder.contains(coord + Vector2I(1, 1)) &&
			s_terrainMapHolder.contains(coord + Vector2I(-1, -1)) && s_terrainMapHolder.contains(coord + Vector2I(1, -1)))
		{
			BlendEdge(s_terrainMapHolder[coord], s_terrainMapHolder[coord + Vector2I(-1, 0)],
				s_terrainMapHolder[coord + Vector2I(1, 0)], s_terrainMapHolder[coord + Vector2I(0, 1)],
				s_terrainMapHolder[coord + Vector2I(0, -1)], s_terrainMapHolder[coord + Vector2I(-1, 1)],
				s_terrainMapHolder[coord + Vector2I(1, 1)], s_terrainMapHolder[coord + Vector2I(-1, -1)],
				s_terrainMapHolder[coord + Vector2I(1, -1)]);
		}
	}
	s_mapMutex.unlock();
	return outOpt;
}

//void TerrainMapGenerator::BlendEdge(TerrainMap& centerMap, const TerrainMap& leftMap, const TerrainMap& rightMap, const TerrainMap& upMap, const TerrainMap& downMap, const TerrainMap& leftUpMap, const TerrainMap& righUptMap, const TerrainMap& leftDownMap, const TerrainMap& rightDownMap)
void TerrainMapGenerator::BlendEdge(TerrainMap& centerMap, TerrainMap &leftMap, TerrainMap &rightMap, TerrainMap &upMap, TerrainMap &downMap, TerrainMap& leftUpMap, TerrainMap& rightUpMap, TerrainMap& leftDownMap, TerrainMap& rightDownMap)
{
	/*Timer timer;*/
	assert(!centerMap.blendedEdges);
	assert(centerMap.width == centerMap.height && centerMap.width == upMap.width); //all should be tested but this should be enugh
	int size = centerMap.width;
	float f;


	int _00 = 0;
	int _10 = size - 1;
	int _01 = size * (size - 1);
	int _11 = size * size - 1;

	float cornerAvg;

	cornerAvg = (centerMap.heightMap[_00] + leftMap.heightMap[_10] +
		 upMap.heightMap[_01] + leftUpMap.heightMap[_11]) * 0.25f;

	centerMap.heightMap[_00] = cornerAvg;
	leftMap.heightMap[_10] = cornerAvg;
	upMap.heightMap[_01] = cornerAvg;
	leftUpMap.heightMap[_11] = cornerAvg;

	cornerAvg = (rightMap.heightMap[_00] + centerMap.heightMap[_10] +
		rightUpMap.heightMap[_01] + upMap.heightMap[_11]) * 0.25f;

	rightMap.heightMap[_00] = cornerAvg;
	centerMap.heightMap[_10] = cornerAvg;
	rightUpMap.heightMap[_01] = cornerAvg;
	upMap.heightMap[_11] = cornerAvg;


	cornerAvg = (rightDownMap.heightMap[_00] + downMap.heightMap[_10] +
		rightMap.heightMap[_01] + centerMap.heightMap[_11]) * 0.25f;

	rightDownMap.heightMap[_00] = cornerAvg;
	downMap.heightMap[_10] = cornerAvg;
	rightMap.heightMap[_01] = cornerAvg;
	centerMap.heightMap[_11] = cornerAvg;

	cornerAvg = (downMap.heightMap[_00] + leftDownMap.heightMap[_10] +
		centerMap.heightMap[_01] + leftMap.heightMap[_11]) * 0.25f;

	downMap.heightMap[_00] = cornerAvg;
	leftDownMap.heightMap[_10] = cornerAvg;
	centerMap.heightMap[_01] = cornerAvg;
	leftMap.heightMap[_11] = cornerAvg;


	//upBlend
	f = upMap.blendedEdges ? 1.0f : 0.5f;
	for (int i = 1; i < size - 1; i++)
	{
		centerMap.heightMap[i] += f * (upMap.heightMap[size * (size - 1) + i] - centerMap.heightMap[i]);

		centerMap.heightMap[i + size] += 0.5f * (centerMap.heightMap[i] - centerMap.heightMap[i + size]);
		centerMap.heightMap[i + 2*size] += 0.5f * (centerMap.heightMap[i+size] - centerMap.heightMap[i + 2*size]);
		centerMap.heightMap[i + 3*size] += 0.5f * (centerMap.heightMap[i+2*size] - centerMap.heightMap[i + 3*size]);
		centerMap.heightMap[i + 4*size] += 0.5f * (centerMap.heightMap[i+3*size] - centerMap.heightMap[i + 4*size]);
	}

	//rightBlend
	f = rightMap.blendedEdges ? 1.0f : 0.5f;
	for (int i = 1; i < size - 1; i++)
	{
		centerMap.heightMap[i * size + size - 1] += f * (rightMap.heightMap[i * size] - centerMap.heightMap[i * size + size - 1]);

		centerMap.heightMap[i * size + size - 2] += 0.5f * (centerMap.heightMap[i * size + size - 1] - centerMap.heightMap[i * size + size - 2]);
		centerMap.heightMap[i * size + size - 3] += 0.5f * (centerMap.heightMap[i * size + size - 2] - centerMap.heightMap[i * size + size - 3]);
		centerMap.heightMap[i * size + size - 4] += 0.5f * (centerMap.heightMap[i * size + size - 3] - centerMap.heightMap[i * size + size - 4]);
		centerMap.heightMap[i * size + size - 5] += 0.5f * (centerMap.heightMap[i * size + size - 4] - centerMap.heightMap[i * size + size - 5]);
	}

	//downBlend
	f = downMap.blendedEdges ? 1.0f : 0.5f;
	for (int i = 1; i < size - 1; i++)
	{
		centerMap.heightMap[size * (size - 1) + i] += f * (downMap.heightMap[i] - centerMap.heightMap[size * (size - 1) + i]);
		
		centerMap.heightMap[size * (size - 2) + i] += 0.5f * (centerMap.heightMap[size * (size - 1) + i] - centerMap.heightMap[size * (size - 2) + i]);
		centerMap.heightMap[size * (size - 3) + i] += 0.5f * (centerMap.heightMap[size * (size - 2) + i] - centerMap.heightMap[size * (size - 3) + i]);
		centerMap.heightMap[size * (size - 4) + i] += 0.5f * (centerMap.heightMap[size * (size - 3) + i] - centerMap.heightMap[size * (size - 4) + i]);
		centerMap.heightMap[size * (size - 5) + i] += 0.5f * (centerMap.heightMap[size * (size - 4) + i] - centerMap.heightMap[size * (size - 5) + i]);
	}

	//leftBlend
	f = leftMap.blendedEdges ? 1.0f : 0.5f;
	for (int i = 1; i < size - 1; i++)
	{
		centerMap.heightMap[i * size] += f * (leftMap.heightMap[i * size + size - 1] - centerMap.heightMap[i * size]);

		centerMap.heightMap[i * size + 1] += 0.5f * (centerMap.heightMap[i * size] - centerMap.heightMap[i * size + 1]);
		centerMap.heightMap[i * size + 2] += 0.5f * (centerMap.heightMap[i * size + 1] - centerMap.heightMap[i * size + 2]);
		centerMap.heightMap[i * size + 3] += 0.5f * (centerMap.heightMap[i * size + 2] - centerMap.heightMap[i * size + 3]);
		centerMap.heightMap[i * size + 4] += 0.5f * (centerMap.heightMap[i * size + 3] - centerMap.heightMap[i * size + 4]);
	}
	
	centerMap.blendedEdges = true;
}




std::vector<float> TerrainMapGenerator::GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
	rfm::Vector2 offset, uint32_t seed)
{
	assert(width > 0 && height > 0 && scale > 0 && octaves >= 1 && lacunarity >= 1 && 0 <= persistance && persistance <= 1);

	Vector2* octRandOffsets = new Vector2[octaves]();
	for (int i = 0; i < octaves; i++)
	{
		octRandOffsets[i].x = GenRandFloat(-10000, 10000, seed + i);
		octRandOffsets[i].y = GenRandFloat(-10000, 10000, octRandOffsets[i].x);
		octRandOffsets[i].x += offset.x;
		octRandOffsets[i].y -= offset.y;
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
				float sampleX = ((float)x + octRandOffsets[i].x - width / 2.0f) * frequency / scale;
				float sampleY = ((float)y + octRandOffsets[i].y - height / 2.0f) * frequency / scale;

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
	float maxHeight = siv::perlin_detail::MaxAmplitude(octaves, persistance); 

	for (int i = 0; i < height * width; i++)
	{
		float normNoise = std::max(0.0f ,(noise[i] + 1) / (2 * maxHeight / 2.0f));
		//normNoise = std::clamp(normNoise, 0.0f, std::numeric_limits<float>::max());
		noise[i] = normNoise;
 		//noise[i] = rfm::InvLerp(minNoise, maxNoise, noise[i]);
	}
	

	
	delete[] octRandOffsets;
	return noise;
}