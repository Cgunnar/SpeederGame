#include "pch.hpp"
#include "TerrainChunk.h"
#include "StandardComponents.h"
#include "AssetManager.h"
#include "RenderComponents.h"
#include "RfextendedMath.hpp"
#include "TerrainMapGenerator.h"
#include "TerrainMeshGenerator.h"
#include "FrameTimer.hpp"

using namespace rfe;
using namespace rfm;

rfm::Vector3 NearestPointOnEdgeFromPoint(rfm::Vector3 corners[4], rfm::Vector3 p);

TerrainChunk::~TerrainChunk()
{
	for (auto& l : m_lodMeshes)
	{
		while (l->waitingOnMesh && !l->hasMesh)
		{
			std::this_thread::yield();
		}
	}
	TerrainMapGenerator::RemoveTerrainMap(m_coord);
	//AssetManager::Get().RemoveTexture2D(m_material.baseColorTexture);
}

TerrainChunk::TerrainChunk(rfm::Vector2I coord, int size, Transform terrainTransform, TerrainMeshDesc mapDesc, std::vector<LODinfo> lods)
	: m_coord(coord), m_lods(lods), m_meshDesc(mapDesc), m_chunkSize(size)
{
	for (const auto& lod : m_lods)
	{
		m_lodMeshes.emplace_back(std::make_unique<TerrainLODMesh>(lod.lod));
	}

	m_position = size * (Vector2)coord;
	//the order matters, corners need to be adjacent to each others
	m_corners[0] = Vector3(m_position.x - size / 2, 0, m_position.y - size / 2);
	m_corners[1] = Vector3(m_position.x + size / 2, 0, m_position.y - size / 2);
	m_corners[2] = Vector3(m_position.x + size / 2, 0, m_position.y + size / 2);
	m_corners[3] = Vector3(m_position.x - size / 2, 0, m_position.y + size / 2);

	float scale = terrainTransform.getScale().x;
	m_chunkEntity = EntityReg::CreateEntity();
	m_chunkEntity.AddComponent<TransformComp>()->transform;
	UpdateChunkTransform(terrainTransform);


	m_material.baseColorFactor = Vector4(Pow(Vector3(0.2), 2.2f), 1);
	m_material.emissiveFactor = 0;
	m_material.flags |= RenderFlag::sampler_anisotropic_clamp | RenderFlag::pixel_shader_terrain; /*| RenderFlag::wireframe*/;
	RenderUnit terranRendUnit;
	terranRendUnit.material = m_material;
	auto rc = m_chunkEntity.AddComponent<RenderUnitComp>();
	rc->unitID = AssetManager::Get().AddRenderUnit(terranRendUnit);
	rc->visible = false;
}

void TerrainChunk::Update(rfm::Vector2 viewPos, float maxViewDist)
{
	if (m_shouldBeRemoved) return;
	if (m_hasMap)
	{
		Vector3 viewPos3D = Vector3(viewPos.x, 0, viewPos.y);
		float viewDist = (viewPos3D - NearestPointOnEdgeFromPoint(m_corners, viewPos3D)).length();

		m_visible = viewDist <= maxViewDist;

		if (m_visible)
		{

			assert(std::is_sorted(m_lods.begin(), m_lods.end(),
				[](auto a, auto b) {return a.visDistThrhold < b.visDistThrhold; }));

			int lodIndex = 0;
			for (int i = 0; i < m_lods.size() - 1; i++)
			{
				if (viewDist > m_lods[i].visDistThrhold)
					lodIndex = i + 1;
				else
					break;
			}
			if (lodIndex != m_prevLODindex)
			{
				auto& lodMesh = m_lodMeshes[lodIndex];
				if (lodMesh->hasMesh)
				{
					if (!lodMesh->hasRenderMesh) lodMesh->GenerateRenderMesh();
					m_chunkEntity.GetComponent<RenderUnitComp>()->ReplaceMesh(lodMesh->renderMesh);
					if (m_prevLODindex != -1)
					{
						m_lodMeshes[m_prevLODindex]->Reset();
					}
					m_prevLODindex = lodIndex;
					m_waitingOnNewLodMesh = false;
				}
				else if (!lodMesh->waitingOnMesh)
				{
					auto map = TerrainMapGenerator::GetTerrainMap(m_coord);
					if (map)
					{
						lodMesh->RequestMesh(*map, m_meshDesc);
						m_waitingOnNewLodMesh = true;
					}
					if (m_prevLODindex == -1) m_visible = false;
				}
				else
				{
					if (m_prevLODindex == -1) m_visible = false;
				}
			}
		}
		else if (viewDist > 1.2f * maxViewDist)
		{
			bool safeToRemove = true;
			for (auto& l : m_lodMeshes)
			{
				safeToRemove = safeToRemove && !l->waitingOnMesh;
			}
			m_shouldBeRemoved = safeToRemove;
		}
		m_chunkEntity.GetComponent<RenderUnitComp>()->visible = m_visible;
	}
	else
	{
		static double deltaTime = 0;
		static double timeStep = 0.5;
		deltaTime += FrameTimer::dt();

		if (deltaTime > timeStep)
		{
			deltaTime = 0;
			auto optMap = TerrainMapGenerator::GetTerrainMap(m_coord);
			m_hasMap = (bool)optMap;
		}
	}
}

void TerrainChunk::LoadTerrain(const TerrainMapDesc& desc)
{
	TerrainMapDesc d = desc;
	d.offset += m_position;
	TerrainMapGenerator::AsyncGenerateTerrinMap(d, m_coord);
}

void TerrainChunk::UpdateChunkTransform(rfm::Transform transform)
{
	auto& chunkTransform = m_chunkEntity.GetComponent<TransformComp>()->transform;
	chunkTransform = transform;
	Transform T;
	T.setTranslation(m_position.x, 0, m_position.y);
	chunkTransform = chunkTransform * T;
}

Triangle TerrainChunk::TriangleAtLocation(Vector3 pos)
{
	if (m_lodMeshes.empty() || !m_lodMeshes[0]->hasTriangles)
	{
		std::cout << "lod 0 has not loaded" << std::endl;
		return Triangle();
	}

	const Transform& chunkTransform = m_chunkEntity.GetComponent<TransformComp>()->transform;
	Vector3 toLocalSpae = inverse(chunkTransform) * Vector4(pos, 1);
	Vector2 localPos = { toLocalSpae.x, toLocalSpae.z };
	int px = static_cast<int>(m_chunkSize / 2 + localPos.x);
	int py = static_cast<int>(m_chunkSize / 2 - localPos.y);
	if (px == m_chunkSize) px = m_chunkSize - 1;
	if (py == m_chunkSize) py = m_chunkSize - 1;
	assert(px < m_chunkSize&& py < m_chunkSize);
	int index = 2 * (py * m_chunkSize + px);
	Triangle t0 = m_lodMeshes[0]->mesh.triangles[index];
	Triangle t1 = m_lodMeshes[0]->mesh.triangles[index + 1];
	Vector2 topLeft = { t0[0].x, t0[0].z };
	Vector2 botRight = { t1[1].x, t1[1].z };

	Triangle tri;
	if ((localPos - topLeft).length() <= (localPos - botRight).length())
		tri = t0;
	else
		tri = t1;

	tri[0] = chunkTransform * Vector4(tri[0], 1);
	tri[1] = chunkTransform * Vector4(tri[1], 1);
	tri[2] = chunkTransform * Vector4(tri[2], 1);

	return tri;
}

rfm::Vector3 NearestPointOnEdgeFromPoint(rfm::Vector3 corners[4], rfm::Vector3 p)
{
	Vector3 cPoint0 = rfm::closestPointOnLineSegmentFromPoint(corners[0], corners[1], p);
	Vector3 cPoint1 = rfm::closestPointOnLineSegmentFromPoint(corners[1], corners[2], p);
	Vector3 cPoint2 = rfm::closestPointOnLineSegmentFromPoint(corners[2], corners[3], p);
	Vector3 cPoint3 = rfm::closestPointOnLineSegmentFromPoint(corners[3], corners[0], p);
	if ((p - cPoint1).length() < (p - cPoint0).length()) cPoint0 = cPoint1;
	if ((p - cPoint2).length() < (p - cPoint0).length()) cPoint0 = cPoint2;
	if ((p - cPoint3).length() < (p - cPoint0).length()) cPoint0 = cPoint3;
	return cPoint0;
}
