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

TerrainChunk::TerrainChunk(rfm::Vector2I coord, int size) : m_coord(coord)
{
	static int i = 0;
	std::cout << "newChunk " << i++ << std::endl;
	m_position = size * (Vector2)coord;
	//m_position = (size+ 0.4f) * (Vector2)coord;
	m_botLeft = Vector3(m_position.x - size/2, 0, m_position.y - size / 2 );
	m_botRight = Vector3( m_position.x + size/2, 0, m_position.y - size / 2 );
	m_topLeft = Vector3( m_position.x - size/2, 0, m_position.y + size / 2 );
	m_topRight = Vector3( m_position.x + size/2, 0, m_position.y + size / 2 );
	m_position = 0.01f * m_position;
	m_chunkEntity = EntityReg::CreateEntity();
	m_chunkEntity.AddComponent<TransformComp>()->transform.setTranslation(Vector3(m_position.x, 0, m_position.y));
	//m_chunkEntity.GetComponent<TransformComp>()->transform.setScale(size/2);
	m_chunkEntity.GetComponent<TransformComp>()->transform.setScale(0.01f);
	//m_chunkEntity.GetComponent<TransformComp>()->transform.setRotationDeg(90, 0, 0);

	auto rc = m_chunkEntity.AddComponent<RenderModelComp>(AssetManager::Get().AddRenderUnit(AssetManager::Get().GetMesh(SimpleMesh::Quad_POS_NOR_UV), Material()));
	auto& m = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
	m.material.materialVariant.baseColorFactor = { 1,1,1 };
	m.material.materialVariant.emissiveFactor = {1,1,1};
	//rc->visible = false;

	
}

void TerrainChunk::Update(rfm::Vector2 viewPos, float maxViewDist)
{
	Vector3 viewPos3D = Vector3(viewPos.x, 0, viewPos.y);
	
	Vector3 closestPoint1 = rfm::closestPointOnLineSegmentFromPoint(m_botLeft, m_botRight, viewPos3D);
	Vector3 closestPoint2 = rfm::closestPointOnLineSegmentFromPoint(m_botLeft, m_topLeft, viewPos3D);
	Vector3 closestPoint3 = rfm::closestPointOnLineSegmentFromPoint(m_botRight, m_topRight, viewPos3D);
	Vector3 closestPoint4 = rfm::closestPointOnLineSegmentFromPoint(m_topLeft, m_topRight, viewPos3D);

	m_visible = false;
	if ((viewPos3D - closestPoint1).length() < maxViewDist) m_visible = true;
	else if ((viewPos3D - closestPoint2).length() < maxViewDist) m_visible = true;
	else if ((viewPos3D - closestPoint3).length() < maxViewDist) m_visible = true;
	else if ((viewPos3D - closestPoint4).length() < maxViewDist) m_visible = true;

	if (m_checkForLoadedTerrainMap)
	{
		auto optMap = TerrainMapGenerator::GetTerrainMap(m_coord);
		if (optMap)
		{
			Material terrainMat;
			terrainMat.baseColorTexture = AssetManager::Get().LoadTex2DFromMemoryR8G8B8A8(
				optMap->colorMapRGBA.data(), optMap->width, optMap->height, LoadTexFlag::GenerateMips);
			terrainMat.emissiveFactor = 0;
			auto rc = m_chunkEntity.GetComponent<RenderModelComp>();
			auto& rendUnit = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
			rendUnit.material = MaterialVariant(terrainMat);
			m_checkForLoadedTerrainMap = false;

			TerrainMeshDesc meshDesc;
			meshDesc.heightScaleFunc = [](float in) {return in <= 0.3f ? 0.3f * 0.3f : in * in; };
			meshDesc.LOD = 2;

			TerrainMeshGenerator::AsyncCreateTerrainMesh(*optMap, [&](TerrainMesh&& mesh){
					OnReceive(std::move(mesh));
				}, meshDesc);
			
		}
	}

	if (m_createRenderMesh)
	{
		auto rc = m_chunkEntity.GetComponent<RenderModelComp>();
		auto& rendUnit = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
		SubMesh terrainMesh(m_mesh.verticesTBN, m_mesh.indices);
		rendUnit.subMesh = terrainMesh;
		m_createRenderMesh = false;
	}
	
	auto rc = m_chunkEntity.GetComponent<RenderModelComp>();// ->visible = m_visible;

	auto& m = AssetManager::Get().GetRenderUnit(rc->renderUnitID);
	if (m_visible)
	{
		m.material.materialVariant.baseColorFactor = { 1,1,1 };
		m.material.materialVariant.emissiveFactor = { 0,0,0 };
	}
	else
	{
		m.material.materialVariant.baseColorFactor = { 1,0,0 };
		m.material.materialVariant.emissiveFactor = { 1,0,0 };
	}
}

void TerrainChunk::LoadTerrain(const TerrainMapDesc& desc)
{
	TerrainMapGenerator::AsyncGenerateTerrinMap(desc, m_coord);
	m_checkForLoadedTerrainMap = true;
}

void TerrainChunk::OnReceive(TerrainMesh&& mesh)
{
	m_createRenderMesh = true;
	m_mesh = std::move(mesh);
}

TerrainLODMesh::TerrainLODMesh(int lod) : m_lod(lod)
{

}

void TerrainLODMesh::OnReceive(TerrainMesh mesh)
{
	m_hasMesh = true;
}

void TerrainLODMesh::RequestMesh(TerrainMap map)
{
	m_hasRequestedMesh = true;
}
