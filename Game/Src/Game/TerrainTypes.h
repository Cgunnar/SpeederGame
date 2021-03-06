#pragma once
#include <vector>
#include <string>
#include "RimfrostMath.hpp"
#include "GraphicsUtilityTypes.h"
#include "GraphicsResources.h"

//constexpr int chunkSize = 1681; //max lod = 8
constexpr int chunkSize = 241; //max lod = 6
//constexpr int chunkSize = 841; //max lod = 7
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
	int erosionIterations = 10000;
	rfm::Vector2 offset;
	uint32_t seed = 123456u;
	float normalizationFactor = 1.0f;
	float hFparam0 = 0;
	float hFparam1 = 0;
	std::function<float(float, const TerrainMapDesc&)> heightScaleFunc = [](float s, const TerrainMapDesc& d) { return s; };
};

struct TerrainMap
{
	bool debugBool = false;
	bool blendedUp = false;
	bool blendedDown = false;
	bool blendedLeft = false;
	bool blendedRight = false;
	bool blendedTopRightCorner = false;
	bool blendedBotRightCorner = false;
	bool blendedTopLeftCorner = false;
	bool blendedBotLeftCorner = false;
	int width, height;
	std::vector<float> heightMap;
};

struct TerrainMeshDesc
{
	float heightScale = 10;
	int LOD = 0;
	rfm::Vector2 uvScale = { 0,0 }; //set to 0,0 to use width, height
	float funktionParmK = 0;
	std::function<float(float, float)> heightScaleFunc = [](float s, float k) { return s; };
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
	~TerrainLODMesh();
	void OnReceive(TerrainMesh&& mesh);
	void RequestMesh(const TerrainMap& map, TerrainMeshDesc desc);
	void GenerateRenderMesh(MeshFormat format = MeshFormat::POS_NOR_UV_TAN_BITAN);
	void Reset();
	TerrainMesh mesh;
	MeshFormat meshFormat = MeshFormat::POS_NOR_UV_TAN_BITAN;
	GID renderMesh;
	bool waitingOnMesh = false;
	bool hasMesh = false;
	bool hasRenderMesh = false;
	bool hasTriangles = false;
	int GetLod() const;
private:
	int m_lod;
	std::mutex m_mutex;
};

struct LODinfo
{
	int lod;
	float visDistThrhold;
};


struct TerrainDesc
{
	TerrainMapDesc map;
	TerrainMeshDesc mesh;
	std::vector<LODinfo> LODs;
	//float frequencyScale = 10;
	//float heightScale = 10;
	//int octaves = 1;
	//float persistence = 0.5f;
	//float lacunarity = 1;
	//int erosionIterations = 10000;
	//rfm::Vector2 baseOffset;
	//uint32_t seed = 123456u;
	//std::vector<Biom> bioms;
	//rfm::Vector2 uvScale = { 0,0 }; //set to 0,0 to use width, height
	//float funktionParmK = 0;
	//std::function<float(float, float)> meshHeightScaleFunc = [](float s, float k) { return s; };
	//std::function<float(float, const TerrainMapDesc&)> mapHeightScaleFunc = [](float s, const TerrainMapDesc& d) { return s; };
};