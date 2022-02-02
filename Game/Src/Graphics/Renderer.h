#pragma once
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "PbrRenderer.h"
#include "SharedRendererResources.h"
#include "RenderComponents.h"
#include "SkyBox.h"
#include "ShadowMappingPass.h"
#include "SpriteRendere.h"

class Renderer
{
	
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void RenderBegin(rfe::Entity& camera);
	void RenderSkyBox(SkyBox& sky);
	void Render(rfe::Entity& camera, DirectionalLight dirLight);
	static SharedRenderResources& GetSharedRenderResources();
private:
	static std::shared_ptr<SharedRenderResources> s_sharedRenderResources;

	void CopyFromECS();
	void SubmitToRender();
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
	std::vector<RendCompAndTransform> m_rendCompAndTransformFromECS;
	std::unordered_map<RenderFlag, std::vector<RendUnitIDAndTransform>> m_renderPassesFlagged;

	SpriteRendere m_spriteRenderer;
	PbrRenderer m_pbrRenderer;
	ShadowMappingPass m_shadowPass;

	VP m_vp;
};

