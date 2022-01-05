#pragma once
#include <vector>
#include <string>
#include "RimfrostMath.hpp"
#include "GraphicsUtilityTypes.h"

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

struct TerrainMapDesc
{
	float scale;
	int octaves;
	float persistence;
	float lacunarity;
	rfm::Vector2 offset;
	uint32_t seed = 123456u;
	std::vector<Biom> bioms;
};

struct TerrainMap
{
	int width, height;
	std::vector<float> heightMap;
	std::vector<uint8_t> colorMapRGBA;
};

struct TerrainMeshDesc
{
	float heightScale = 10;
	int LOD = 0;
	rfm::Vector2 uvScale = { 0,0 }; //set to 0,0 to use width, height
	std::function<float(float)> heightScaleFunc = [](float s) { return s; };
};


struct TerrainMesh
{
	static constexpr uint32_t vertexStride = sizeof(Vertex_POS_NOR_UV);
	static constexpr uint32_t vertexStrideTBN = sizeof(Vertex_POS_NOR_UV_TAN_BITAN);
	std::vector<Triangle> triangles;
	std::vector<Vertex_POS_NOR_UV> vertices;
	std::vector<Vertex_POS_NOR_UV_TAN_BITAN> verticesTBN;
	std::vector<uint32_t> indices;
};