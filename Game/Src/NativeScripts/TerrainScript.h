#pragma once
#include "NativeScript.h"
#include "TerrainChunk.h"

class TerrainScript : public rfe::NativeScriptComponent<TerrainScript>
{
public:
	TerrainScript() = default;
	~TerrainScript();
	TerrainScript(TerrainDesc desc);
	void OnStart();
	void OnUpdate(float dt);
	Triangle GetTriangleAtPos(rfm::Vector3 pos);
private:
	void UpdateChunks(rfm::Vector2 viewPos);
	float m_maxViewDistance = 0;
	std::queue<rfm::Vector2I> m_chunksToLoad;
	TerrainMapDesc m_mapDesc;
	TerrainMeshDesc m_meshDesc;
	int m_chunkMeshSize = 0;
	int m_chunksVisibleInViewDist = 0;
	std::unordered_map<rfm::Vector2I, TerrainChunk*> m_chunkMap;
	std::vector<rfm::Vector2I> m_prevFrameVisibleChunksCoord;
	std::vector<LODinfo> m_lods;
};

