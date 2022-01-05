#include "pch.hpp"
#include "TerrainScript.h"
#include "TerrainMapGenerator.h"
#include "RenderComponents.h"


using namespace rfm;
using namespace rfe;





TerrainScript::~TerrainScript()
{
	for (auto c : m_chunkMap)
	{
		delete c.second;
	}
	m_chunkMap.clear();
}

TerrainScript::TerrainScript(TerrainMapDesc desc) : m_mapDesc(desc)
{	
	m_lods.push_back({ .lod = 0, .visDistThrhold = 100 });
	m_lods.push_back({ .lod = 1, .visDistThrhold = 300 });
	m_lods.push_back({ .lod = 2, .visDistThrhold = 400 });

	m_maxViewDistance = std::max_element(m_lods.begin(), m_lods.end(), [](LODinfo lodA, LODinfo lodB) {
		return lodA.visDistThrhold < lodB.visDistThrhold; })->visDistThrhold;
}

void TerrainScript::OnStart()
{
	m_chunkSize = TerrainMapGenerator::chunkSize - 1;
	float s = GetComponent<TransformComp>()->transform.getScale().x;
	//m_chunkSize *= s;
	m_chunksVisibleInViewDist = static_cast<int>(round(m_maxViewDistance / m_chunkSize));
}

void TerrainScript::OnUpdate(float dt)
{
	EntityID viewerID = EntityReg::GetComponentArray<PlayerComp>().front().GetEntity();
	Transform viewerTransform = EntityReg::GetComponent<TransformComp>(viewerID)->transform;

	Vector2 viewerPos = Vector2(viewerTransform.getTranslation().x, viewerTransform.getTranslation().z) / 0.01f;
	UpdateChunks(viewerPos);
}

void TerrainScript::UpdateChunks(rfm::Vector2 viewPos)
{
	Vector2I chunkCoord;
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkSize));


	for (auto& c : m_prevFrameVisibleChunksCoord)
	{
		auto rc = m_chunkMap[c]->m_chunkEntity.GetComponent<RenderModelComp>();// ->visible = vis;

		auto& m = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
		m.material.materialVariant.baseColorFactor = { 0,0,0 };
		m.material.materialVariant.emissiveFactor = { 0,0,0 };
	}
	m_prevFrameVisibleChunksCoord.clear();

	if (!m_chunksToLoad.empty())
	{
		m_chunkMap[m_chunksToLoad.front()]->LoadTerrain(m_mapDesc);
		m_chunksToLoad.pop();
	}

	for (int y = -m_chunksVisibleInViewDist; y <= m_chunksVisibleInViewDist; y++)
	{
		for (int x = -m_chunksVisibleInViewDist; x <= m_chunksVisibleInViewDist; x++)
		{
			Vector2I viewedChunk = chunkCoord + Vector2I(x, y);
			if (m_chunkMap.contains(viewedChunk))
			{
				m_chunkMap[viewedChunk]->Update(viewPos, m_maxViewDistance);
				if(m_chunkMap[viewedChunk]->m_visible) m_prevFrameVisibleChunksCoord.push_back(viewedChunk);
			}
			else
			{
				m_chunkMap[viewedChunk] = new TerrainChunk(viewedChunk, m_chunkSize, m_lods);
				m_chunksToLoad.emplace(viewedChunk);
			}
			
		}
	}
}
