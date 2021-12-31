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

	void SetModel(GID modelID)
	{
		ModelID = modelID;
		Model& model = AssetManager::Get().GetModel(modelID);
		renderUnitBegin = model.RenderUnitBegin;
		renderUnitEnd = model.RenderUnitEnd;
	}
	void SetRenderUnit(RenderUnitID renderUnitID)
	{
		ModelID = 0; //invalid
		renderUnitBegin = renderUnitID;
		renderUnitEnd = renderUnitID + 1;
	}

	RenderPassEnum renderPass = RenderPassEnum::none;
	GID ModelID;
	RenderUnitID renderUnitID = 0;
	RenderUnitID renderUnitBegin = 0, renderUnitEnd = 0;
};

struct IndexedMeshComp: rfe::Component<IndexedMeshComp>
{
	IndexBuffer indexBuffer;
	VertexBuffer vertexBuffer;
};

struct PointLightComp : rfe::Component<PointLightComp>
{
	PointLight pointLight;
};

struct DirectionalLightComp : rfe::Component<DirectionalLightComp>
{
	DirectionalLight dirLight;
};