#include "pch.hpp"
#include "TerrainGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"

inline static constexpr unsigned char ToUint8(float x) noexcept
{
	return (x <= 0.0) ? 0 : (1.0 <= x) ? 255 : static_cast<unsigned char>(x * 255.0 + 0.5);
}

float* TerrainGenerator::GenerateNoise(int width, int height, float scale, uint32_t seed)
{
	assert(width > 0 && height > 0 && scale > 0);
	const siv::PerlinNoise::seed_type sivseed = seed;
	const siv::PerlinNoise perlin{ sivseed };

	float* noise = new float[(size_t)width * (size_t)height]();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float sampleX = x / scale;
			float sampleY = y / scale;

			noise[y * width + x] = static_cast<float>(perlin.octave2D_01(sampleX, sampleY, 1));
		}
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

	return noise;
}
