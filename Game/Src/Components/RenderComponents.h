#pragma once
#include "rfEntity.hpp"
#include "utilityTypes.h"
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "AssetManager.h"
#include "StandardComponents.h"

struct RenderUnitComp : rfe::Component<RenderUnitComp>
{
	RenderUnitComp() = default;
	RenderUnitComp(GID meshID, const Material& material)
	{
		unitID = AssetManager::Get().AddRenderUnit(meshID, material);
	}
	RenderUnitComp(Mesh mesh, const Material& material)
	{
		unitID = AssetManager::Get().AddRenderUnit(mesh, material);
	}
	/*void SetRenderUnit(GID meshID, const Material& material, bool visible = true)
	{
		unitID = AssetManager::Get().AddRenderUnit(meshID, material);
		this->visible = visible;
	}*/
	RenderUnitID unitID = 0;
	RenderPassEnum renderPass;
	bool visible = true;
};

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
	RendCompAndTransform() = default;
	RendCompAndTransform(rfm::Transform worldMatrix, RenderPassEnum renderPass, RenderUnitID id, RenderUnitID begin = 0, RenderUnitID end = 0)
		: worldMatrix(worldMatrix), renderPass(renderPass), begin(begin), end(end), id(id) {}
	RenderPassEnum renderPass;
	RenderUnitID begin = 0, end = 0, id = 0;
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