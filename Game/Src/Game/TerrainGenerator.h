#pragma once

#include "RimfrostMath.hpp"
#include "TerrainMeshGenerator.h"

struct Biom
{
	Biom() = default;
	Biom(const std::string& name, rfm::Vector3 color, float threshold, bool terrainIsFlat = false) :
		name(name), color(color), threshold(threshold), flat(terrainIsFlat) {}
	std::string name = "";
	rfm::Vector3 color = 0;
	float threshold = 0;
	bool flat = false;
};





class TerrainGenerator
{
public:
	

	TerrainMap GenerateTerrinMap(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed = 123456u);


	std::vector<Biom> bioms;

private:
	std::vector<float> GenerateNoise(int width, int height, float scale, int octaves, float persistance, float lacunarity,
		rfm::Vector2 offset, uint32_t seed = 123456u);


	static constexpr int chunkSize = 241;
};

