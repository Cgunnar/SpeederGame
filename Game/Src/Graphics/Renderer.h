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
#include "Scene.h"

class Renderer
{
	
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void RenderBegin(rfe::Entity& camera);
	void RenderSkyBox(SkyBox& sky);
	void RenderScene(Scene& scene);
	void RenderToEnvMap(rfm::Vector3 position, Scene& scene, uint32_t res, SkyBox* sky = nullptr);
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
	const float m_farPlane = 7000.0f;
	const float m_nearPlane = 0.01f;

	VP m_vp;
};

