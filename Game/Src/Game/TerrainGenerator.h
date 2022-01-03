#pragma once

#include "RimfrostMath.hpp"

class TerrainGenerator
{
public:
	struct Bioms
	{
		std::string name;
		rfm::Vector3 color = 0;
		float height = 0;
	};

	float* GenerateTerrinMap(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed = 123456u);


	std::vector<Bioms> bioms;

private:
	float* GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed = 123456u);
};

