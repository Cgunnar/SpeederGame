#include "pch.hpp"
#include "TerrainGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"

using namespace rfm;

inline static constexpr unsigned char ToUint8(float x) noexcept
{
	return (x <= 0.0) ? 0 : (1.0 <= x) ? 255 : static_cast<unsigned char>(x * 255.0 + 0.5);
}






float* TerrainGenerator::GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
	uint32_t seed, rfm::Vector2 offset)
{
	Vector2* octRandOffsets = new Vector2[octaves]();
	for (int i = 0; i < octaves; i++)
	{
		octRandOffsets[i].x = GenRandFloat(-10000, 10000, seed + octaves) + offset.x;
		octRandOffsets[i].y = GenRandFloat(-10000, 10000, octRandOffsets[i].x) + offset.y;
	}

	assert(width > 0 && height > 0 && scale > 0);
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
				float sampleX = x * frequency / scale + octRandOffsets[i].x;
				float sampleY = y * frequency / scale + octRandOffsets[i].y;

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
	

	unsigned char* noiseChar = new unsigned char[(size_t)width * (size_t)height]();
	for (int i = 0; i < height * width; i++)
	{
		noiseChar[i] = ToUint8(noise[i]);
	}

	if (!stbi_write_bmp("testNoise.bmp", width, height, STBI_grey, noiseChar))
	{
		std::cout << "write error" << std::endl;
	}
	delete[] noiseChar;
	delete[] octRandOffsets;
	return noise;
}
