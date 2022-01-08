#pragma once
#include <vector>
#include <string>
#include "RimfrostMath.hpp"
#include "GraphicsUtilityTypes.h"
#include "GraphicsResources.h"

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
	float frequencyScale = 10;
	int octaves = 1;
	float persistence = 0.5f;
	float lacunarity = 1;
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


class TerrainLODMesh
{
public:
	TerrainLODMesh(int lod);
	TerrainLODMesh() = default;
	void OnReceive(TerrainMesh&& mesh);
	void RequestMesh(const TerrainMap& map, TerrainMeshDesc desc);
	void GenerateRenderMesh(MeshFormat format = MeshFormat::POS_NOR_UV_TAN_BITAN);
	TerrainMesh mesh;
	MeshFormat meshFormat = MeshFormat::POS_NOR_UV_TAN_BITAN;
	Mesh renderMesh;
	bool hasRequestedMesh = false;
	bool hasMesh = false;
	bool hasRenderMesh = false;
private:
	int m_lod;
};

struct LODinfo
{
	int lod;
	float visDistThrhold;
};


struct TerrainDesc
{
	float frequencyScale = 10;
	float heightScale = 10;
	int octaves = 1;
	float persistence = 0.5f;
	float lacunarity = 1;
	rfm::Vector2 baseOffset;
	uint32_t seed = 123456u;
	std::vector<Biom> bioms;
	std::vector<LODinfo> LODs;
	rfm::Vector2 uvScale = { 0,0 }; //set to 0,0 to use width, height
	std::function<float(float)> heightScaleFunc = [](float s) { return s; };
};