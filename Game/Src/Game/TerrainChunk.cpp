#include "pch.hpp"
#include "TerrainChunk.h"
#include "StandardComponents.h"
#include "AssetManager.h"
#include "RenderComponents.h"
#include "RfextendedMath.hpp"
#include "TerrainMapGenerator.h"
#include "TerrainMeshGenerator.h"

using namespace rfe;
using namespace rfm;

rfm::Vector3 NearestPointOnEdgeFromPoint(rfm::Vector3 corners[4], rfm::Vector3 p);

TerrainChunk::~TerrainChunk()
{
	for (auto& l : m_lodMeshes)
	{
		while (l.hasRequestedMesh && !l.hasMesh)
		{
			std::this_thread::yield();
		}
	}
	AssetManager::Get().RemoveTexture2D(m_material.baseColorTexture);
}

TerrainChunk::TerrainChunk(rfm::Vector2I coord, int size, Transform terrainTransform, TerrainMeshDesc mapDesc, std::vector<LODinfo> lods)
	: m_coord(coord), m_lods(lods), m_meshDesc(mapDesc), m_chunkSize(size)
{
	for (const auto& lod : m_lods)
	{
		m_lodMeshes.emplace_back(lod.lod);
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


	m_material.baseColorFactor = 1;
	m_material.emissiveFactor = 0;
	m_material.flags |= RenderFlag::sampler_anisotropic_clamp;
	auto rc = m_chunkEntity.AddComponent<RenderModelComp>();
	rc->SetRenderUnit(AssetManager::Get().GetMesh(SimpleMesh::Quad_POS_NOR_UV), m_material, false);
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
				if (lodMesh.hasMesh)
				{
					if (!lodMesh.hasRenderMesh) lodMesh.GenerateRenderMesh();
					m_chunkEntity.GetComponent<RenderModelComp>()->SetRenderUnit(lodMesh.renderMesh, m_material);
					m_prevLODindex = lodIndex;
				}
				else if (!lodMesh.hasRequestedMesh)
				{
					lodMesh.RequestMesh(m_map, m_meshDesc);
					if (m_prevLODindex == -1) m_visible = false;
				}
			}
		}
		else if (viewDist > 2 * maxViewDist)
		{
			bool safeToRemove = true;
			for (auto& l : m_lodMeshes)
			{
				safeToRemove = safeToRemove && (l.hasMesh || !l.hasRequestedMesh);
			}
			m_shouldBeRemoved = safeToRemove;
		}
		m_chunkEntity.GetComponent<RenderModelComp>()->visible = m_visible;

	}
	else if (m_checkForLoadedTerrainMap)
	{
		auto optMap = TerrainMapGenerator::GetTerrainMap(m_coord);
		if (optMap)
		{
			std::cout << "LoadedMap\n";
			m_map = *optMap;
			m_hasMap = true;
			m_material.baseColorTexture = AssetManager::Get().LoadTex2DFromMemoryR8G8B8A8(
				optMap->colorMapRGBA.data(), optMap->width, optMap->height, LoadTexFlag::GenerateMips);
			m_material.emissiveFactor = 0;

			m_checkForLoadedTerrainMap = false;
		}
	}
}

void TerrainChunk::LoadTerrain(const TerrainMapDesc& desc)
{
	TerrainMapDesc d = desc;
	d.offset += m_position;
	TerrainMapGenerator::AsyncGenerateTerrinMap(d, m_coord);
	m_checkForLoadedTerrainMap = true;
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
	if (m_lodMeshes.empty() || !m_lodMeshes[0].hasRenderMesh)
	{
		std::cout << "lod 0 has not loaded" << std::endl;
		return Triangle();
	}

	const Transform& chunkTransform = m_chunkEntity.GetComponent<TransformComp>()->transform;
	Vector3 toLocalSpae = inverse(chunkTransform) * Vector4(pos, 1);
	Vector2 localPos = { toLocalSpae.x, toLocalSpae.z };
	int px = static_cast<int>(m_chunkSize / 2 + localPos.x);
	int py = static_cast<int>(m_chunkSize / 2 - localPos.y);

	int index = 2 * (py * m_chunkSize + px);
	Triangle t0 = m_lodMeshes[0].mesh.triangles[index];
	Triangle t1 = m_lodMeshes[0].mesh.triangles[index + 1];
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
