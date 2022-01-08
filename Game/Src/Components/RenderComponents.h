#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "AssetManager.h"
#include "StandardComponents.h"



struct RenderModelComp : rfe::Component<RenderModelComp>
{
	
	RenderModelComp(const std::string& filePath = "", RenderPassEnum renderPass = RenderPassEnum::none, bool visible = true)
	{
		if (!filePath.empty())
		{
			auto modelID = AssetManager::Get().LoadModel(filePath);
			Model& model = AssetManager::Get().GetModel(modelID);
			this->renderPass = renderPass;
			renderUnitBegin = model.RenderUnitBegin;
			renderUnitEnd = model.RenderUnitEnd;
			ModelID = modelID;
			this->visible = visible;
		}
		else
		{
			this->visible = false;
		}
	}

	

	RenderModelComp(RenderUnitID renderUnitID)
	{
		this->SetRenderUnit(renderUnitID);
	}

	
	void SetModel(GID modelID, bool visible = true)
	{
		ModelID = modelID;
		Model& model = AssetManager::Get().GetModel(modelID);
		renderUnitBegin = model.RenderUnitBegin;
		renderUnitEnd = model.RenderUnitEnd;
		this->visible = visible;
	}

	void SetRenderUnit(const Mesh& mesh, const Material& material, bool visible = true)
	{
		renderUnitID = AssetManager::Get().AddRenderUnit(mesh.GetGID(), material);
		renderUnitBegin = 0;
		renderUnitEnd = 0;
		ModelID = 0;
		this->visible = visible;
	}
	void SetRenderUnit(GID meshID, const Material& material, bool visible = true)
	{
		renderUnitID = AssetManager::Get().AddRenderUnit(meshID, material);
		renderUnitBegin = 0;
		renderUnitEnd = 0;
		ModelID = 0;
		this->visible = visible;
	}

	void SetRenderUnit(RenderUnitID id, bool visible = true)
	{
		renderUnitID = id;
		renderUnitBegin = 0;
		renderUnitEnd = 0;
		ModelID = 0;
		this->visible = visible;
	}

	RenderPassEnum renderPass = RenderPassEnum::none;
	GID ModelID;
	RenderUnitID renderUnitID = 0;
	RenderUnitID renderUnitBegin = 0, renderUnitEnd = 0;
	bool visible = true;
};

struct RendCompAndTransform
{
	RenderModelComp rendComp;
	rfm::Transform worldMatrix;
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