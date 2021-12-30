#pragma once

#include <vector>
#include "LowLvlGfx.h"
#include "GraphicsResources.h"
#include "SharedRendererResources.h"
#include "rfEntity.hpp"

class PhongRenderer
{
public:
	PhongRenderer(std::weak_ptr<SharedRenderResources> sharedRes);
	PhongRenderer() = default;
	~PhongRenderer() = default;

	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type);
	void PreProcess(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);
	void Render(const VP& viewAndProjMatrix, rfe::Entity& camera, RenderFlag flag);

private:
	void RenderWithColorOnly(RenderFlag flag);
	void RenderWithDiffuseTexture(RenderFlag flag);
	void RenderPhongMaterial_DiffTex_NormTex(RenderFlag flag);
	void RenderPhongMaterial_DiffTex_NormTex_SpecTex(RenderFlag flag);


	bool m_prePocessed = false;

	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;

	Shader m_PS_Phong_singlePointLight;
	Shader m_PS_Phong_DiffTexture_singleLight;
	Shader m_PS_Phong_DiffTex_NorTex_singleLight;
	Shader m_PS_Phong_DiffTex_NorTex_SpecTex_pointLight;
	ConstantBuffer m_phongMaterialCB;
	Sampler m_anisotropic_wrapSampler;


	/*struct alignas(16) MatrixAl16
	{
		rfm::Matrix m;
	};*/

	

	//std::vector<RendUnitIDAndTransform> m_unitsToRender;
	std::vector<RendUnitIDAndTransform> m_colorUnits;
	std::vector<RendUnitIDAndTransform> m_diffTextureUnits;
	std::vector<RendUnitIDAndTransform> m_PhongMaterial_DiffTex_NormTex;
	std::vector<RendUnitIDAndTransform> m_PhongMaterial_DiffTex_NormTex_SpecTex;
};

