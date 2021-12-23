#pragma once
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "PhongRenderer.h"
#include "PbrRenderer.h"
#include "SharedRendererResources.h"
#include "RenderComponents.h"
#include "SkyBox.h"

class Renderer
{
	
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void RenderBegin(rfe::Entity& camera);
	void RenderSkyBox(SkyBox& sky);
	void Render(rfe::Entity& camera);
	static SharedRenderResources& GetSharedRenderResources();
private:
	static std::shared_ptr<SharedRenderResources> s_sharedRenderResources;

	void SubmitToRender(rfe::Entity& camera);
	void SubmitToInternalRenderers(AssetManager& am, RenderPassEnum renderPass, RenderUnitID unitID, const rfm::Transform& worldMatrix);
	//void SubmitOpaqueToInternalRenderers(RenderPassEnum renderPass, RenderUnitID unitID, const rfm::Transform& worldMatrix, MaterialType type);
	void SubmitAndRenderTransparentToInternalRenderers(const VP& viewAndProjMatrix, rfe::Entity& camera);

	void RenderAllPasses(const VP& viewAndProjMatrix, rfe::Entity& camera);
	void SetUpHdrRTV();

	struct PassForTransparentUnits
	{
		RenderFlag rendFlag;
		RenderPassEnum rendPass;
		RendUnitIDAndTransform unit;
	};
	std::vector<PassForTransparentUnits> m_transparentRenderUnits;
	std::unordered_map<RenderFlag, std::vector<RendUnitIDAndTransform>> m_renderPassesFlagged;

	PhongRenderer m_phongRenderer;
	PbrRenderer m_pbrRenderer;

	VP m_vp;
};

