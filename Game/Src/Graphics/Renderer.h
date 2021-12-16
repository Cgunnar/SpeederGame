#pragma once
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "PhongRenderer.h"
#include "PbrRenderer.h"
#include "SharedRendererResources.h"
#include "RenderComponents.h"

class Renderer
{
	
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void Render(rfe::Entity& camera);

private:
	std::shared_ptr<SharedRenderResources> m_sharedRenderResources;

	void SubmitToRender(rfe::Entity& camera);
	void SubmitToInternalRenderers(RenderPassEnum renderPass, RenderUnitID unitID, const rfm::Transform& worldMatrix, MaterialType type);
	void SetUpHdrRTV();

	struct PassForTransparentUnits
	{
		RenderPassEnum rendPass;
		RendUnitIDAndTransform unit;
	};
	std::vector<PassForTransparentUnits> m_transparentRenderUnits;

	PhongRenderer m_phongRenderer;
	PbrRenderer m_pbrRenderer;

	VP m_vp;
};

