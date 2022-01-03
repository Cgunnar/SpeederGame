#include "pch.hpp"
#include "Terrain.h"
#include "StandardComponents.h"
#include "AssetManager.h"
#include "RenderComponents.h"
#include "RfextendedMath.hpp"

using namespace rfe;
using namespace rfm;

TerrainChunk::TerrainChunk(rfm::Vector2I coord, int size)
{

	m_position = (size + 2.1) * (Vector2)coord;
	m_botLeft = { m_position.x - size/2, 0, m_position.y - size / 2 };
	m_botRight = { m_position.x + size/2, 0, m_position.y - size / 2 };
	m_topLeft = { m_position.x - size/2, 0, m_position.y + size / 2 };
	m_topRight = { m_position.x + size/2, 0, m_position.y + size / 2 };

	m_terrainMesh = EntityReg::CreateEntity();
	m_terrainMesh.AddComponent<TransformComp>()->transform.setTranslation(m_position.x, 0, m_position.y);
	m_terrainMesh.GetComponent<TransformComp>()->transform.setScale(size);
	m_terrainMesh.GetComponent<TransformComp>()->transform.setRotationDeg(90, 0, 0);

	auto rc = m_terrainMesh.AddComponent<RenderModelComp>(AssetManager::Get().AddRenderUnit(AssetManager::Get().GetMesh(SimpleMesh::Quad_POS_NOR_UV), Material()));
	rc->visible = false;
}

void TerrainChunk::Update(rfm::Vector2 viewPos, float maxViewDist)
{
	Vector3 viewPos3D = { viewPos.x, 0, viewPos.y };
	
	Vector3 closestPoint1 = rfm::closestPointOnLineSegmentFromPoint(m_botLeft, m_botRight, viewPos3D);
	Vector3 closestPoint2 = rfm::closestPointOnLineSegmentFromPoint(m_botLeft, m_topLeft, viewPos3D);
	Vector3 closestPoint3 = rfm::closestPointOnLineSegmentFromPoint(m_botRight, m_topRight, viewPos3D);
	Vector3 closestPoint4 = rfm::closestPointOnLineSegmentFromPoint(m_topLeft, m_topRight, viewPos3D);

	bool vis = false;
	if ((viewPos3D - closestPoint1).length() <= maxViewDist) vis = true;
	else if ((viewPos3D - closestPoint2).length() <= maxViewDist) vis = true;
	else if ((viewPos3D - closestPoint3).length() <= maxViewDist) vis = true;
	else if ((viewPos3D - closestPoint4).length() <= maxViewDist) vis = true;

	m_terrainMesh.GetComponent<RenderModelComp>()->visible = vis;
}
