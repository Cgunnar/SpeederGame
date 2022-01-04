#include "pch.hpp"
#include "TerrainScript.h"
#include "TerrainGenerator.h"
#include "RenderComponents.h"


using namespace rfm;
using namespace rfe;





TerrainScript::TerrainScript(TerrainMapDesc desc, uint32_t seed) : m_seed(seed), m_mapDesc(desc)
{

}

void TerrainScript::OnStart()
{
	m_chunkSize = TerrainMapGenerator::chunkSize - 1;
	float s = GetComponent<TransformComp>()->transform.getScale().x;
	m_chunkSize *= s;
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
	
	float s = GetComponent<TransformComp>()->transform.getScale().x;
	Vector2I chunkCoord;
	chunkCoord.x = round(viewPos.x / m_chunkSize);
	chunkCoord.y = round(viewPos.y / m_chunkSize);


	for (auto& c : m_prevFrameVisibleChunksCoord)
	{
		auto rc = m_chunkMap[c].m_terrainMesh.GetComponent<RenderModelComp>();// ->visible = vis;

		auto& m = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
		m.material.materialVariant.baseColorFactor = { 0,0,0 };
		m.material.materialVariant.emissiveFactor = { 0,0,0 };
	}
	m_prevFrameVisibleChunksCoord.clear();

	if (!m_chunksToLoad.empty())
	{
		m_chunkMap[m_chunksToLoad.front()].LoadTerrain(m_mapDesc);
		m_chunksToLoad.pop();
	}

	for (int y = -m_chunksVisibleInViewDist; y <= m_chunksVisibleInViewDist; y++)
	{
		for (int x = -m_chunksVisibleInViewDist; x <= m_chunksVisibleInViewDist; x++)
		{
			Vector2I viewedChunk = chunkCoord + Vector2I(x, y);
			if (m_chunkMap.contains(viewedChunk))
			{
				m_chunkMap[viewedChunk].Update(viewPos, viewDistance);
				if(m_chunkMap[viewedChunk].m_visible) m_prevFrameVisibleChunksCoord.push_back(viewedChunk);
			}
			else
			{
				m_chunkMap[viewedChunk] = TerrainChunk(viewedChunk, m_chunkSize);
				m_chunksToLoad.emplace(viewedChunk);
				
			}
			
		}
	}
}
