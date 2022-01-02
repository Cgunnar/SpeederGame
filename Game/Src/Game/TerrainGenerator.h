#pragma once

#include "RimfrostMath.hpp"

class TerrainGenerator
{
public:

	float* GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		uint32_t seed, rfm::Vector2 offset);
};

