#include "pch.hpp"
#include "TerrainGenerator.h"

#include "PerlinNoise.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "RimfrostMath.hpp"
#include "RandGen.hpp"
#include "UtilityFunctions.h"

using namespace rfm;


TerrainMap TerrainGenerator::GenerateTerrinMap(int width, int height, float scale, int octaves, float persistance, float lacunarity, rfm::Vector2 offset, uint32_t seed)
{

	TerrainMap map;
	map.height = height;
	map.width = width;
	map.heightMap = GenerateNoise(width, height, scale, octaves, persistance, lacunarity, offset, seed);
	
	
	//unsigned char* noiseChar = new unsigned char[(size_t)width * (size_t)height]();
	//for (int i = 0; i < height * width; i++)
	//{
	//	noiseChar[i] = util::ToUint8(map.heightMap[i]);
	//}
	//if (!stbi_write_bmp("testNoise.bmp", width, height, STBI_grey, noiseChar))
	//{
	//	std::cout << "write error" << std::endl;
	//}
	//delete[] noiseChar;


	std::vector<Vector4> colorMap;
	colorMap.resize(height * (size_t)width);
	for (int i = 0; i < height * width; i++)
	{
		for (auto& b : bioms)
		{
			if (map.heightMap[i] <= b.threshold)
			{
				if(b.flat) map.heightMap[i] = b.threshold;
				colorMap[i] = Vector4(b.color, 1);
				break;
			}
		}
	}

	map.colorMapRGBA = util::FloatToCharRGBA((float*)colorMap.data(), width, height);
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
		/*noise[i] *= 3 * noise[i];
 		noise[i] = rfm::InvLerp(0, maxNoise * 3, noise[i]);*/
	}
	

	
	delete[] octRandOffsets;
	return noise;
}
