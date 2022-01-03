#include "pch.hpp"
#include "TerrainScript.h"
#include "TerrainGenerator.h"

using namespace rfm;
using namespace rfe;

TerrainScript::TerrainScript(uint32_t seed) : m_seed(seed)
{

}

void TerrainScript::OnStart()
{
	m_chunkSize = TerrainGenerator::chunkSize - 1;
	m_chunksVisibleInViewDist = round(viewDistance / m_chunkSize);
}

void TerrainScript::OnUpdate(float dt)
{
	EntityID viewerID = EntityReg::GetComponentArray<PlayerComp>().front().GetEntity();
	Transform viewerTransform = EntityReg::GetComponent<TransformComp>(viewerID)->transform;

	Vector2 viewerPos = { viewerTransform.getTranslation().x, viewerTransform.getTranslation().z };
	UpdateChunks(viewerPos);
}

void TerrainScript::UpdateChunks(rfm::Vector2 viewPos)
{
	
	Vector2I chunkCoord;
	chunkCoord.x = round(viewPos.x / m_chunkSize);
	chunkCoord.y = round(viewPos.y / m_chunkSize);

	for (int y = -m_chunksVisibleInViewDist; y <= m_chunksVisibleInViewDist; y++)
	{
		for (int x = -m_chunksVisibleInViewDist; x <= m_chunksVisibleInViewDist; x++)
		{
			Vector2I viewedChunk = chunkCoord + Vector2I(x, y);
			float s = GetComponent<TransformComp>()->transform.getScale().x;
			if (m_chunkMap.contains(viewedChunk))
			{
				m_chunkMap[viewedChunk].Update(viewPos, s*viewDistance);
			}
			else
			{
				
				m_chunkMap[viewedChunk] = TerrainChunk(viewedChunk, s*m_chunkSize);
			}
		}
	}
}
