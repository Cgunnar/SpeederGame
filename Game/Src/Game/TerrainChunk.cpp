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

TerrainChunk::TerrainChunk(rfm::Vector2I coord, int size, float scale, std::vector<LODinfo> lods)
	: m_coord(coord), m_lods(lods)
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

	m_chunkEntity = EntityReg::CreateEntity();
	m_chunkEntity.AddComponent<TransformComp>()->transform.setTranslation(scale * Vector3(m_position.x, 0, m_position.y));
	m_chunkEntity.GetComponent<TransformComp>()->transform.setScale(scale);

	m_material.name = "TerrainChunk " + std::to_string(coord.x) + ", " + std::to_string(coord.y);
	m_material.baseColorFactor = 1;
	m_material.emissiveFactor = 0;
	//m_material.flags |= RenderFlag::wireframe;
	//m_material.flags |= RenderFlag::sampler_anisotropic_clamp;
	m_material.flags |= RenderFlag::sampler_point_clamp;
	auto rc = m_chunkEntity.AddComponent<RenderModelComp>();
	rc->SetRenderUnit(AssetManager::Get().GetMesh(SimpleMesh::Quad_POS_NOR_UV), m_material, false);
}

void TerrainChunk::Update(rfm::Vector2 viewPos, float maxViewDist, float scale)
{
	auto transform = m_chunkEntity.GetComponent<TransformComp>();
	transform->transform.setTranslation(scale * Vector3(m_position.x, 0, m_position.y));
	transform->transform.setScale(scale);
	if (m_hasMap)
	{
		Vector3 viewPos3D = Vector3(viewPos.x, 0, viewPos.y);
		float viewDist = (viewPos3D - NearestPointOnEdgeFromPoint(m_corners, viewPos3D)).length();

		m_visible = viewDist <= maxViewDist;

		if (m_visible)
		{
			m_chunkEntity.GetComponent<RenderModelComp>()->visible = true;
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
					lodMesh.RequestMesh(m_map);
				}
			}
		}
	}
	else if (m_checkForLoadedTerrainMap)
	{
		auto optMap = TerrainMapGenerator::GetTerrainMap(m_coord);
		if (optMap)
		{
			m_map = *optMap;
			m_hasMap = true;
			m_material.baseColorTexture = AssetManager::Get().LoadTex2DFromMemoryR8G8B8A8(
				optMap->colorMapRGBA.data(), optMap->width, optMap->height, LoadTexFlag::GenerateMips);
			m_material.emissiveFactor = 0;

			m_checkForLoadedTerrainMap = false;
		}
	}

	auto rc = m_chunkEntity.GetComponent<RenderModelComp>();// ->visible = m_visible;


	auto& m = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
	if (m_visible)
	{
		m.material.baseColorFactor = { 1,1,1,1};
		m.material.emissiveFactor = { 0,0,0 };
	}
	else
	{
		m.material.baseColorFactor = { 1,0,0,1 };
		m.material.emissiveFactor = { 1,0,0 };
	}
}

void TerrainChunk::LoadTerrain(const TerrainMapDesc& desc)
{
	TerrainMapDesc d = desc;
	d.offset = m_position;
	TerrainMapGenerator::AsyncGenerateTerrinMap(d, m_coord);
	m_checkForLoadedTerrainMap = true;
}

void TerrainChunk::OnReceive(TerrainMesh&& mesh)
{
	m_createRenderMesh = true;
	m_mesh = std::move(mesh);
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
