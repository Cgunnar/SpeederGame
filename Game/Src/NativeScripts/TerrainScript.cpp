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

TerrainScript::TerrainScript(TerrainDesc desc)
{	
	m_mapDesc.bioms = desc.bioms;
	m_mapDesc.frequencyScale = desc.frequencyScale;
	m_mapDesc.lacunarity = desc.lacunarity;
	m_mapDesc.octaves = desc.octaves;
	m_mapDesc.persistence = desc.persistence;
	m_mapDesc.offset = desc.baseOffset;
	m_mapDesc.seed = desc.seed;

	m_meshDesc.heightScale = desc.heightScale;
	m_meshDesc.heightScaleFunc = desc.heightScaleFunc;
	m_meshDesc.uvScale = desc.uvScale;
	
	if (desc.LODs.empty())
	{
		m_lods.push_back({ .lod = 0, .visDistThrhold = 200 });
	}
	else
	{
		m_lods = desc.LODs;
	}

	m_maxViewDistance = std::max_element(m_lods.begin(), m_lods.end(), [](LODinfo lodA, LODinfo lodB) {
		return lodA.visDistThrhold < lodB.visDistThrhold; })->visDistThrhold;
}

void TerrainScript::OnStart()
{
	m_chunkSize = TerrainMapGenerator::chunkSize - 1;
	m_chunksVisibleInViewDist = static_cast<int>(round(m_maxViewDistance / m_chunkSize));
}

void TerrainScript::OnUpdate(float dt)
{
	EntityID viewerID = EntityReg::GetComponentArray<PlayerComp>().front().GetEntity();
	Transform viewerTransform = EntityReg::GetComponent<TransformComp>(viewerID)->transform;
	
	UpdateChunks({ viewerTransform.getTranslation().x, viewerTransform.getTranslation().z });
}

Triangle TerrainScript::GetTriangleAtPos(Vector2 pos)
{

	Vector2I chunkCoord;
	float s = GetComponent<TransformComp>()->transform.getScale().x;
	Vector2 viewPos = pos / s;
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkSize));
	if (!m_chunkMap.contains(chunkCoord))
	{
		std::cout << "chunk does not exist, return hight above y=0" << std::endl;
		return Triangle();
	}

	TerrainChunk *chunk = m_chunkMap.at(chunkCoord);
	Triangle triLocalToChunk = chunk->TriangleAtLocation(pos - s*chunk->m_position);
	triLocalToChunk[0] += s * Vector3(chunk->m_position.x, 0, chunk->m_position.y);
	triLocalToChunk[1] += s * Vector3(chunk->m_position.x, 0, chunk->m_position.y);
	triLocalToChunk[2] += s * Vector3(chunk->m_position.x, 0, chunk->m_position.y);
	return triLocalToChunk;
}

void TerrainScript::UpdateChunks(rfm::Vector2 viewPos)
{
	float s = GetComponent<TransformComp>()->transform.getScale().x;
	viewPos /= s;

	Vector2I chunkCoord;
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkSize));


	for (auto& c : m_prevFrameVisibleChunksCoord)
	{
		m_chunkMap[c]->m_chunkEntity.GetComponent<RenderModelComp>()->visible = false;
	}
	m_prevFrameVisibleChunksCoord.clear();

	std::vector<Vector2I> removeChunks;
	for (auto& it : m_chunkMap)
	{
		m_chunkMap[it.first]->Update(viewPos, m_maxViewDistance);
		m_chunkMap[it.first]->UpdateChunkTransform(GetComponent<TransformComp>()->transform);
		if (it.second->m_shouldBeRemoved)
		{
			removeChunks.push_back(it.first);
		}
		else
		{
			m_prevFrameVisibleChunksCoord.push_back(it.first);
		}
	}
	for (auto& c : removeChunks)
	{
		delete m_chunkMap[c];
		m_chunkMap.erase(c);
	}

	if (!m_chunksToLoad.empty())
	{
		m_chunkMap[m_chunksToLoad.front()]->LoadTerrain(m_mapDesc);
		m_chunkMap[m_chunksToLoad.front()]->Update(viewPos, m_maxViewDistance);
		m_chunksToLoad.pop();
	}

	for (int y = -m_chunksVisibleInViewDist; y <= m_chunksVisibleInViewDist; y++)
	{
		for (int x = -m_chunksVisibleInViewDist; x <= m_chunksVisibleInViewDist; x++)
		{
			Vector2I viewedChunk = chunkCoord + Vector2I(x, y);
			if (!m_chunkMap.contains(viewedChunk))
			{
				m_chunkMap[viewedChunk] = new TerrainChunk(viewedChunk, m_chunkSize, GetComponent<TransformComp>()->transform, m_meshDesc, m_lods);
				m_chunksToLoad.emplace(viewedChunk);
			}
		}
	}
}
