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
	m_mapDesc.erosionIterations = desc.erosionIterations;

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
	if (!TerrainMapGenerator::IsInitialized()) TerrainMapGenerator::Init();
	m_chunkMeshSize = chunkSize - 1;
	m_chunksVisibleInViewDist = static_cast<int>(round(m_maxViewDistance / m_chunkMeshSize));
}

void TerrainScript::OnUpdate(float dt)
{
	EntityID viewerID = EntityReg::GetComponentArray<CameraComp>().front().GetEntity();
	const Transform& viewerTransform = EntityReg::GetComponent<TransformComp>(viewerID)->transform;
	const Transform& terrainTransform = GetComponent<TransformComp>()->transform;

	Vector3 viewPosInTerrainSpace = inverse(terrainTransform) * Vector4(viewerTransform.getTranslation(), 1);
	UpdateChunks({ viewPosInTerrainSpace.x, viewPosInTerrainSpace.z });
}

Triangle TerrainScript::GetTriangleAtPos(Vector3 pos)
{
	Transform terrainTransform = GetComponent<TransformComp>()->transform;
	Vector3 localPos = inverse(terrainTransform) * Vector4(pos, 1);
	Vector2I chunkCoord;
	Vector2 viewPos = {localPos.x, localPos.z};
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkMeshSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkMeshSize));
	if (!m_chunkMap.contains(chunkCoord))
	{
		std::cout << "chunk does not exist, return hight above y=0" << std::endl;
		return Triangle();
	}

	TerrainChunk *chunk = m_chunkMap.at(chunkCoord);
	Triangle triLocalToChunk = chunk->TriangleAtLocation(pos);
	return triLocalToChunk;
}

void TerrainScript::UpdateChunks(rfm::Vector2 viewPos)
{
	Vector2I chunkCoord;
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkMeshSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkMeshSize));
	Transform transform = GetComponent<TransformComp>()->transform;

	for (auto& c : m_prevFrameVisibleChunksCoord)
	{
		m_chunkMap[c]->m_chunkEntity.GetComponent<RenderModelComp>()->visible = false;
	}
	m_prevFrameVisibleChunksCoord.clear();

	std::vector<Vector2I> removeChunks;
	for (auto& [coord, chunk] : m_chunkMap)
	{
		m_chunkMap[coord]->Update(viewPos, m_maxViewDistance);
		m_chunkMap[coord]->UpdateChunkTransform(transform);
		if (chunk->m_shouldBeRemoved)
		{
			removeChunks.push_back(coord);
		}
		else
		{
			m_prevFrameVisibleChunksCoord.push_back(coord);
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

	
	for (int y = 0; y <= m_chunksVisibleInViewDist + 1; y++)
	{
		for (int x = 0; x <= m_chunksVisibleInViewDist + 1; x++)
		{
			Vector2I viewedChunk0 = chunkCoord + Vector2I(x, y);
			Vector2I viewedChunk1 = chunkCoord + Vector2I(-x, y);
			Vector2I viewedChunk2 = chunkCoord + Vector2I(x, -y);
			Vector2I viewedChunk3 = chunkCoord + Vector2I(-x, -y);
			if (!m_chunkMap.contains(viewedChunk0))
			{
				m_chunkMap[viewedChunk0] = new TerrainChunk(viewedChunk0, m_chunkMeshSize, transform, m_meshDesc, m_lods);
				m_chunksToLoad.emplace(viewedChunk0);
			}
			if (y == 0 && x == 0) continue;
			if (!m_chunkMap.contains(viewedChunk1))
			{
				m_chunkMap[viewedChunk1] = new TerrainChunk(viewedChunk1, m_chunkMeshSize, transform, m_meshDesc, m_lods);
				m_chunksToLoad.emplace(viewedChunk1);
			}
			if (!m_chunkMap.contains(viewedChunk2))
			{
				m_chunkMap[viewedChunk2] = new TerrainChunk(viewedChunk2, m_chunkMeshSize, transform, m_meshDesc, m_lods);
				m_chunksToLoad.emplace(viewedChunk2);
			}
			if (!m_chunkMap.contains(viewedChunk3))
			{
				m_chunkMap[viewedChunk3] = new TerrainChunk(viewedChunk3, m_chunkMeshSize, transform, m_meshDesc, m_lods);
				m_chunksToLoad.emplace(viewedChunk3);
			}
		}
	}
}
