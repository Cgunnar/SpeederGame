#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "AssetManager.h"



struct RenderModelComp : rfe::Component<RenderModelComp>
{
	
	RenderModelComp(const std::string& filePath = "", RenderPassEnum renderPass = RenderPassEnum::none)
	{
		if (!filePath.empty())
		{
			auto modelID = AssetManager::Get().LoadModel(filePath);
			Model& model = AssetManager::Get().GetModel(modelID);
			this->renderPass = renderPass;
			renderUnitBegin = model.RenderUnitBegin;
			renderUnitEnd = model.RenderUnitEnd;
			ModelID = modelID;
		}
	}
	

	RenderPassEnum renderPass = RenderPassEnum::none;
	GID ModelID;
	RenderUnitID renderUnitID = 0;
	RenderUnitID renderUnitBegin = 0, renderUnitEnd = 0;
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