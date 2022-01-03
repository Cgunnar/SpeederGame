#include "pch.hpp"
#include "TerrainGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "UtilityFunctions.h"

using namespace rfm;





float* TerrainGenerator::GenerateTerrinMap(int width, int height, float scale, int octaves, float persistance, float lacunarity, rfm::Vector2 offset, uint32_t seed)
{
	float* noise = GenerateNoise(width, height, scale, octaves, persistance, lacunarity, offset, seed);
	
	
	unsigned char* noiseChar = new unsigned char[(size_t)width * (size_t)height]();
	for (int i = 0; i < height * width; i++)
	{
		noiseChar[i] = util::ToUint8(noise[i]);
	}

	if (!stbi_write_bmp("testNoise.bmp", width, height, STBI_grey, noiseChar))
	{
		std::cout << "write error" << std::endl;
	}
	delete[] noiseChar;


	std::vector<Vector3> colorMap;
	colorMap.resize(height * (size_t)width);
	for (int i = 0; i < height * width; i++)
	{
		for (auto& b : bioms)
		{
			if (noise[i] <= b.height)
			{
				colorMap[i] = b.color;
				break;
			}
		}
	}

	auto colorUint8 = util::FloatToCharRGB((float*)colorMap.data(), width, height);

	if (!stbi_write_png("terrainColor.png", width, height, STBI_rgb, colorUint8.data(), width*STBI_rgb))
	{
		std::cout << "write error" << std::endl;
	}


	return noise;
}

float* TerrainGenerator::GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
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

	float* noise = new float[(size_t)width * (size_t)height]();

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

			noise[y * width + x] = noiseHeight;
		}
	}
	for (int i = 0; i < height * width; i++)
	{
 		noise[i] = rfm::InvLerp(minNoise, maxNoise, noise[i]);
	}
	

	
	delete[] octRandOffsets;
	return noise;
}
