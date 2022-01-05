#pragma once
#include "NativeScript.h"
#include "TerrainChunk.h"
#include <thread>

class TerrainScript : public rfe::NativeScriptComponent<TerrainScript>
{
public:
	TerrainScript() = default;
	~TerrainScript();
	TerrainScript(TerrainMapDesc desc, uint32_t seed = 32);
	void OnStart();
	void OnUpdate(float dt);

	static constexpr float viewDistance = 800;
private:
	void UpdateChunks(rfm::Vector2 viewPos);

	//std::queue<std::function<void(TerrainMap)>> m_callbackTerrainMapQueue;
	uint32_t m_seed = 4324345;
	std::queue<rfm::Vector2I> m_chunksToLoad;
	TerrainMapDesc m_mapDesc;
	int m_chunkSize = 0;
	int m_chunksVisibleInViewDist = 0;
	std::unordered_map<rfm::Vector2I, TerrainChunk*> m_chunkMap;
	std::vector<rfm::Vector2I> m_prevFrameVisibleChunksCoord;
};

