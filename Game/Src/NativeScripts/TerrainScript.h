#pragma once
#include "NativeScript.h"
#include "Terrain.h"

class TerrainScript : public rfe::NativeScriptComponent<TerrainScript>
{
public:
	TerrainScript() = default;
	TerrainScript(uint32_t seed);
	void OnStart();
	void OnUpdate(float dt);

	static constexpr float viewDistance = 20;
private:
	void UpdateChunks(rfm::Vector2 viewPos);


	uint32_t m_seed = 4324345;

	int m_chunkSize = 0;
	int m_chunksVisibleInViewDist = 0;
	std::unordered_map<rfm::Vector2I, TerrainChunk> m_chunkMap;
	std::vector<rfm::Vector2I> m_prevFrameVisibleChunksCoord;
};

