#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"



struct RenderComp : rfe::Component<RenderComp>
{
	enum class RenderPassEnum
	{
		phong = 0,
		phong_transparent,
		end,
	};

	RenderPassEnum renderPass = RenderPassEnum::end;
	GID ModelID;
	RenderUnit renderUnit;
};

//struct DiffuseTexturMaterialComp : rfe::Component<DiffuseTexturMaterialComp>
//{
//	GID textureID;
//	rfm::Vector3 specularColor{ 1,1,1 };
//	float shininess = 700;
//};

struct IndexedMeshComp: rfe::Component<IndexedMeshComp>
{
	IndexBuffer indexBuffer;
	VertexBuffer vertexBuffer;
};

//struct IndexedSubMeshComp : rfe::Component<IndexedSubMeshComp>
//{
//	struct SubMeshOffsets
//	{
//		uint32_t indexCount;
//		uint32_t startIndexLocation;
//		int32_t baseVertexLocation;
//	} meshOffsets;
//};

struct PointLightComp : rfe::Component<PointLightComp>
{
	PointLight pointLight;
};