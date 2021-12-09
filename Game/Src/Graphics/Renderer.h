#pragma once
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"
#include "rfEntity.hpp"
#include "AssetManager.h"

class Renderer
{
	
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void Render(rfe::Entity& camera);

private:

	//void PhongRender(rfe::Entity& camera);

	void RunRenderPasses(rfe::Entity& camera);

	void RunRenderPasses2(rfe::Entity& camera);

	void DrawRenderUnit(RenderUnitID id, const AssetManager& am);

	ConstantBuffer m_worldMatrixCB;
	ConstantBuffer m_pointLightCB;
	ConstantBuffer m_phongMaterialCB;
	ConstantBuffer m_vpCB;

	Shader m_vertexShader;
	Shader m_phongPS;
	Sampler m_sampler;

	struct alignas(16) VP
	{
		rfm::Matrix V;
		rfm::Matrix P;
	} m_vp;
};

