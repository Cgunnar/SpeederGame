#pragma once
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "PhongRenderer.h"
#include "SharedRendererResources.h"

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
	std::vector<RendUnitIDAndTransform> m_transparentRenderUnits;

	PhongRenderer m_phongRenderer;

	VP m_vp;
};

