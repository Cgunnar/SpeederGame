#include "pch.hpp"
#include "TerrainGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "UtilityFunctions.h"

using namespace rfm;


TerrainMap TerrainGenerator::GenerateTerrinMap(TerrainMapDesc mapDesc)
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

std::vector<float> TerrainGenerator::GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
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
