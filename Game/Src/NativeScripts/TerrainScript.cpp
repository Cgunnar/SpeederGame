#include "pch.hpp"
#include "TerrainScript.h"
#include "TerrainMapGenerator.h"
#include "RenderComponents.h"
#include "imgui.h"


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
	/*m_mapDesc.bioms = desc.bioms;
	m_mapDesc.frequencyScale = desc.frequencyScale;
	m_mapDesc.lacunarity = desc.lacunarity;
	m_mapDesc.octaves = desc.octaves;
	m_mapDesc.persistence = desc.persistence;
	m_mapDesc.offset = desc.baseOffset;
	m_mapDesc.seed = desc.seed;
	m_mapDesc.erosionIterations = desc.erosionIterations;
	m_mapDesc.heightScaleFunc = desc.mapHeightScaleFunc;*/
	m_mapDesc = desc.map;

	m_meshDesc = desc.mesh;
	/*m_meshDesc.heightScale = desc.mesh.heightScale;
	m_meshDesc.heightScaleFunc = desc.mesh.heightScaleFunc;
	m_meshDesc.uvScale = desc.mesh.uvScale;*/
	
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



	int visbleChunks = 0;
	int waitingOnMap = 0;
	int waitingOnMesh = 0;
	for (auto& [coord, chunk] : m_chunkMap)
	{
		if (chunk->m_visible) visbleChunks++;
		if (!chunk->m_hasMap) waitingOnMap++;
		if (chunk->m_waitingOnNewLodMesh) waitingOnMesh++;
	}

	ImGui::Begin("TerrainScript");
	ImGui::Text("number of chunks %u", m_chunkMap.size());
	ImGui::Text("waitingOnMap %d", waitingOnMap);
	ImGui::Text("is visable %d", visbleChunks);
	ImGui::Text("waitingOnMesh %d", waitingOnMesh);


	ImGui::End();


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

float TerrainScript::GetHeightOverTerrain(rfm::Vector3 pos)
{
	Triangle tri = GetTriangleAtPos(pos);
	float groundHeight = (dot(tri.normal, tri[0]) - tri.normal.x * pos.x - tri.normal.z * pos.z) / tri.normal.y;
	return pos.y - groundHeight;
}

void TerrainScript::UpdateChunks(rfm::Vector2 viewPos)
{
	Vector2I chunkCoord;
	chunkCoord.x = static_cast<int>(round(viewPos.x / m_chunkMeshSize));
	chunkCoord.y = static_cast<int>(round(viewPos.y / m_chunkMeshSize));
	Transform transform = GetComponent<TransformComp>()->transform;

	for (auto& c : m_prevFrameVisibleChunksCoord)
	{
		m_chunkMap[c]->m_chunkEntity.GetComponent<RenderUnitComp>()->visible = false;
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


	std::vector<Vector2I> chunkCoordsToUpdate;
	for (int y = -m_chunksVisibleInViewDist; y <= m_chunksVisibleInViewDist; y++)
	{
		for (int x = -m_chunksVisibleInViewDist; x <= m_chunksVisibleInViewDist; x++)
		{
			Vector2I coord{ x, y };
			//if(coord.length() < m_chunksVisibleInViewDist + 0.6f) //some number to allow more then 4 new chunks in the max range
				chunkCoordsToUpdate.push_back(coord);
		}
	}
	std::sort(chunkCoordsToUpdate.begin(), chunkCoordsToUpdate.end(), [](auto a, auto b) {
		return a.length() < b.length(); });

	for (auto& c : chunkCoordsToUpdate)
	{
		Vector2I viewedChunk = chunkCoord + c;
		if (!m_chunkMap.contains(viewedChunk))
		{
			m_chunkMap[viewedChunk] = new TerrainChunk(viewedChunk, m_chunkMeshSize, transform, m_meshDesc, m_lods);
			m_chunksToLoad.emplace(viewedChunk);
		}
	}
}
