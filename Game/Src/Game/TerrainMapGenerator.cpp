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
#include "FrameTimer.hpp"

using namespace rfm;

std::mutex TerrainMapGenerator::s_mapMutex;

std::unordered_map<rfm::Vector2I, TerrainMapGenerator::TerrainMapAndMetaData> TerrainMapGenerator::s_terrainMapHolder;
bool TerrainMapGenerator::s_initialized = false;





void TerrainMapGenerator::AsyncGenerateTerrinMapInternal(const TerrainMapDesc& mapDesc, rfm::Vector2I coord, uint64_t frameNumber)
{
	TerrainMap newMap = TerrainMapGenerator::GenerateTerrinMap(mapDesc);
	s_mapMutex.lock();
	if (s_terrainMapHolder.contains(coord))
	{
		uint64_t thisframe = FrameTimer::frame();
		if (thisframe >= s_terrainMapHolder[coord].frameRequested)
		{
			s_terrainMapHolder[coord].map = newMap;
			s_terrainMapHolder[coord].desc = mapDesc;
			s_terrainMapHolder[coord].frameRequested = frameNumber;
		}
	}
	else
	{
		s_terrainMapHolder[coord].map = newMap;
		s_terrainMapHolder[coord].desc = mapDesc;
		s_terrainMapHolder[coord].frameRequested = frameNumber;
	}
	s_mapMutex.unlock();
}


void TerrainMapGenerator::Init()
{
	ErosionSimulator::Init(3);
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
	map.heightMap = GenerateNoise(chunkSize, chunkSize, mapDesc);
#ifdef DEBUG

	ErosionSimulator::Erode(map, std::min(2000, mapDesc.erosionIterations));
#else

	ErosionSimulator::Erode(map, mapDesc.erosionIterations);
#endif // DEBUG


	return map;
}

void TerrainMapGenerator::AsyncGenerateTerrinMap(TerrainMapDesc mapDesc, rfm::Vector2I coord)
{
	s_mapMutex.lock();
	if (s_terrainMapHolder.contains(coord)) //does this need to be locked to prevent ub? No two chunks can have the same coord
	{
		s_terrainMapHolder.erase(coord);
	}
	s_mapMutex.unlock();

	//s_terrainMapHolder[coord] = TerrainMapGenerator::GenerateTerrinMap(mapDesc);
	WorkerThreads::AddTask(TerrainMapGenerator::AsyncGenerateTerrinMapInternal, mapDesc, coord, FrameTimer::frame());
}

std::optional<TerrainMap> TerrainMapGenerator::GetTerrainMap(rfm::Vector2I coord)
{
	std::optional<TerrainMap> outOpt = std::nullopt;
	s_mapMutex.lock();
	if (s_terrainMapHolder.contains(coord))
	{
		auto& map = s_terrainMapHolder[coord].map;
		if (map.blendedBotLeftCorner && map.blendedBotRightCorner && map.blendedTopLeftCorner && map.blendedTopRightCorner &&
			map.blendedDown && map.blendedLeft && map.blendedRight && map.blendedUp)
		{
			map.debugBool = true;
			outOpt = std::make_optional(map);
			//s_terrainMapHolder.erase(coord);

		}
		else
		{
			TerrainMap* botLeft = s_terrainMapHolder.contains(coord + Vector2I(-1, -1)) ? &s_terrainMapHolder[coord + Vector2I(-1, -1)].map : nullptr;
			TerrainMap* botRight = s_terrainMapHolder.contains(coord + Vector2I(1, -1)) ? &s_terrainMapHolder[coord + Vector2I(1, -1)].map : nullptr;
			TerrainMap* topLeft = s_terrainMapHolder.contains(coord + Vector2I(-1, 1)) ? &s_terrainMapHolder[coord + Vector2I(-1, 1)].map : nullptr;
			TerrainMap* topRight = s_terrainMapHolder.contains(coord + Vector2I(1, 1)) ? &s_terrainMapHolder[coord + Vector2I(1, 1)].map : nullptr;
			TerrainMap* left = s_terrainMapHolder.contains(coord + Vector2I(-1, 0)) ? &s_terrainMapHolder[coord + Vector2I(-1, 0)].map : nullptr;
			TerrainMap* right = s_terrainMapHolder.contains(coord + Vector2I(1, 0)) ? &s_terrainMapHolder[coord + Vector2I(1, 0)].map : nullptr;
			TerrainMap* top = s_terrainMapHolder.contains(coord + Vector2I(0, 1)) ? &s_terrainMapHolder[coord + Vector2I(0, 1)].map : nullptr;
			TerrainMap* bot = s_terrainMapHolder.contains(coord + Vector2I(0, -1)) ? &s_terrainMapHolder[coord + Vector2I(0, -1)].map : nullptr;
			
			bool needToWait = false;
			if (!((botLeft && bot && left) || map.blendedBotLeftCorner)) needToWait = true;
			if (!((botRight && bot && right) || map.blendedBotRightCorner)) needToWait = true;
			if (!((topLeft && top && left) || map.blendedTopLeftCorner)) needToWait = true;
			if (!((topRight && top && right) || map.blendedTopRightCorner)) needToWait = true;
			if (!(left || map.blendedLeft)) needToWait = true;
			if (!(right || map.blendedRight)) needToWait = true;
			if (!(top || map.blendedUp)) needToWait = true;
			if (!(bot || map.blendedDown)) needToWait = true;

			if (!needToWait)
			{
				BlendEdge(map, left, right, top, bot, topLeft, topRight, botLeft, botRight);
			}
		}
	}
	s_mapMutex.unlock();
	return outOpt;
}

void TerrainMapGenerator::RemoveTerrainMap(rfm::Vector2I coord)
{
	s_mapMutex.lock();
	if(s_terrainMapHolder.contains(coord))
		s_terrainMapHolder.erase(coord);
	s_mapMutex.unlock();
}

void TerrainMapGenerator::BlendEdge(TerrainMap& centerMap, TerrainMap *leftMap, TerrainMap *rightMap, TerrainMap *upMap, TerrainMap *downMap, TerrainMap* leftUpMap, TerrainMap* rightUpMap, TerrainMap* leftDownMap, TerrainMap* rightDownMap)
{
	int size = centerMap.width;

	int _00 = 0;
	int _10 = size - 1;
	int _01 = size * (size - 1);
	int _11 = size * size - 1;

	if (!centerMap.blendedTopLeftCorner)
	{
		float cornerAvg = (centerMap.heightMap[_00] + leftMap->heightMap[_10] +
			upMap->heightMap[_01] + leftUpMap->heightMap[_11]) * 0.25f;

		centerMap.heightMap[_00] = cornerAvg;
		leftMap->heightMap[_10] = cornerAvg;
		upMap->heightMap[_01] = cornerAvg;
		leftUpMap->heightMap[_11] = cornerAvg;

		centerMap.blendedTopLeftCorner = true;
		leftMap->blendedTopRightCorner = true;
		upMap->blendedBotLeftCorner= true;
		leftUpMap->blendedBotRightCorner = true;
	}

	if (!centerMap.blendedTopRightCorner)
	{
		float cornerAvg = (rightMap->heightMap[_00] + centerMap.heightMap[_10] +
			rightUpMap->heightMap[_01] + upMap->heightMap[_11]) * 0.25f;

		rightMap->heightMap[_00] = cornerAvg;
		centerMap.heightMap[_10] = cornerAvg;
		rightUpMap->heightMap[_01] = cornerAvg;
		upMap->heightMap[_11] = cornerAvg;

		rightMap->blendedTopLeftCorner = true;
		centerMap.blendedTopRightCorner = true;
		rightUpMap->blendedBotLeftCorner = true;
		upMap->blendedBotRightCorner = true;
	}

	if (!centerMap.blendedBotRightCorner)
	{
		float cornerAvg = (rightDownMap->heightMap[_00] + downMap->heightMap[_10] +
			rightMap->heightMap[_01] + centerMap.heightMap[_11]) * 0.25f;

		rightDownMap->heightMap[_00] = cornerAvg;
		downMap->heightMap[_10] = cornerAvg;
		rightMap->heightMap[_01] = cornerAvg;
		centerMap.heightMap[_11] = cornerAvg;

		rightDownMap->blendedTopLeftCorner = true;
		downMap->blendedTopRightCorner = true;
		rightMap->blendedBotLeftCorner = true;
		centerMap.blendedBotRightCorner = true;
	}

	if (!centerMap.blendedBotLeftCorner)
	{
		float cornerAvg = (downMap->heightMap[_00] + leftDownMap->heightMap[_10] +
			centerMap.heightMap[_01] + leftMap->heightMap[_11]) * 0.25f;

		downMap->heightMap[_00] = cornerAvg;
		leftDownMap->heightMap[_10] = cornerAvg;
		centerMap.heightMap[_01] = cornerAvg;
		leftMap->heightMap[_11] = cornerAvg;

		downMap->blendedTopLeftCorner = true;
		leftDownMap->blendedTopRightCorner = true;
		centerMap.blendedBotLeftCorner = true;
		leftMap->blendedBotRightCorner = true;
	}

	constexpr int x = 4;
	//upBlend
	if (!centerMap.blendedUp)
	{
		for (int i = 1; i < size - 1; i++)
		{
			float upFromEdge = upMap->heightMap[size * (size - 1 - x) + i];
			float downFromEdge = centerMap.heightMap[x*size + i];
			float avg = 0.5f*upFromEdge + 0.5f*downFromEdge;

			for (int j = 0; j < x; j++)
			{
				float s = static_cast<float>(j) / static_cast<float>(x);
				centerMap.heightMap[j * size + i] = std::lerp(downFromEdge, avg, 1-s);
				upMap->heightMap[size * (size - 1 - j) + i] = std::lerp(avg, upFromEdge, s);
			}
			centerMap.blendedUp = true;
			upMap->blendedDown = true;
		}
	}

	if (!centerMap.blendedRight)
	{
		//rightBlend
		for (int i = 1; i < size - 1; i++)
		{
			float rightFromEdge = rightMap->heightMap[i * size + x];
			float leftFromEdge = centerMap.heightMap[i * size + size - 1 - x];
			float avg = 0.5f*rightFromEdge + 0.5f*leftFromEdge;
			for (int j = 0; j < x; j++)
			{
				float s = static_cast<float>(j) / static_cast<float>(x);
				centerMap.heightMap[i * size + size - 1 - j] = std::lerp(leftFromEdge, avg, 1 - s);
				rightMap->heightMap[i * size + j] = std::lerp(avg, rightFromEdge, s);
			}
			centerMap.blendedRight = true;
			rightMap->blendedLeft = true;
		}
	}

	if (!centerMap.blendedDown)
	{
		//downBlend
		for (int i = 1; i < size - 1; i++)
		{
			float downFromEdge = downMap->heightMap[x*size + i];
			float upFromEdge = centerMap.heightMap[size * (size - 1 - x) + i];
			float avg = 0.5f*downFromEdge + 0.5f*upFromEdge;
			for (int j = 0; j < x; j++)
			{
				float s = static_cast<float>(j) / static_cast<float>(x);
				centerMap.heightMap[size * (size - 1 - j) + i] = std::lerp(upFromEdge, avg, 1 - s);
				downMap->heightMap[j * size + i] = std::lerp(avg, downFromEdge, s);
			}
			centerMap.blendedDown = true;
			downMap->blendedUp = true;
		}
	}

	if (!centerMap.blendedLeft)
	{
		//leftBlend
		for (int i = 1; i < size - 1; i++)
		{
			float leftFromEdge = leftMap->heightMap[i * size + size - 1 - x];
			float rightFromEdge = centerMap.heightMap[i * size + x];
			float avg = 0.5f*leftFromEdge + 0.5f*rightFromEdge;
			for (int j = 0; j < x; j++)
			{
				float s = static_cast<float>(j) / static_cast<float>(x);
				centerMap.heightMap[i * size + j] = std::lerp(rightFromEdge, avg, 1 - s);
				leftMap->heightMap[i * size + size - 1 - j] = std::lerp(avg, leftFromEdge, s);
			}
			centerMap.blendedLeft = true;
			leftMap->blendedRight = true;
		}
	}
}




std::vector<float> TerrainMapGenerator::GenerateNoise(int width, int height, const TerrainMapDesc& mapDesc)
{
	assert(width > 0 && height > 0 && mapDesc.frequencyScale > 0 && mapDesc.octaves >= 1 && mapDesc.lacunarity >= 1 && 0 <= mapDesc.persistence && mapDesc.persistence <= 1);

	Vector2* octRandOffsets = new Vector2[mapDesc.octaves]();
	for (int i = 0; i < mapDesc.octaves; i++)
	{
		octRandOffsets[i].x = GenRandFloat(-10000, 10000, mapDesc.seed + i);
		octRandOffsets[i].y = GenRandFloat(-10000, 10000, octRandOffsets[i].x);
		octRandOffsets[i].x += mapDesc.offset.x;
		octRandOffsets[i].y -= mapDesc.offset.y;
	}

	const siv::PerlinNoise::seed_type sivseed = mapDesc.seed;
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

			for (int i = 0; i < mapDesc.octaves; i++)
			{
				float sampleX = ((float)x + octRandOffsets[i].x - width / 2.0f) * frequency / mapDesc.frequencyScale;
				float sampleY = ((float)y + octRandOffsets[i].y - height / 2.0f) * frequency / mapDesc.frequencyScale;

				float perlinNoise = amplitude * static_cast<float>(perlin.noise2D(sampleX, sampleY));
				noiseHeight += perlinNoise;
				amplitude *= mapDesc.persistence;
				frequency *= mapDesc.lacunarity;
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
	float maxHeight = siv::perlin_detail::MaxAmplitude(mapDesc.octaves, mapDesc.persistence);

	for (int i = 0; i < height * width; i++)
	{
		float normNoise = std::max(0.0f ,(noise[i] + 1) / (maxHeight * mapDesc.normalizationFactor));
		//float normNoise = std::max(0.0f ,(noise[i] + 1) / (2 * maxHeight / 1.5f));
		//auto f = [](float h, float k) {return h < k ? 0.0f : 1.0f - sqrt(std::max(0.0f, 1.0f - (h - k) * (h - k))); };
		//normNoise = scale / 150  * f(normNoise, 0);
		normNoise = mapDesc.heightScaleFunc(normNoise, mapDesc);
		noise[i] = normNoise;
	}
	

	
	delete[] octRandOffsets;
	return noise;
}