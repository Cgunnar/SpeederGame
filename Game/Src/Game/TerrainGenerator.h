#pragma once



class TerrainGenerator
{
public:

	float* GenerateNoise(int width, int height, float scale, uint32_t seed);
};

