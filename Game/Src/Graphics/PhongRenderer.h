#pragma once

#include <vector>
#include "LowLvlGfx.h"
#include "GraphicsResources.h"
#include "SharedRendererResources.h"

class PhongRenderer
{
public:
	PhongRenderer(std::weak_ptr<SharedRenderResources> sharedRes);
	PhongRenderer() = default;
	~PhongRenderer() = default;

	void Submit(RenderUnitID unitID, const rfm::Transform& worlMatrix, MaterialType type);
	void PreProcess(const VP& viewAndProjMatrix);
	void Render(const VP& viewAndProjMatrix);

private:
	void RenderWithColorOnly();
	void RenderWithDiffuseTexture();


	bool m_prePocessed = false;

	std::weak_ptr<SharedRenderResources> m_sharedRenderResources;

	Shader m_PS_Phong_color_singleLight;
	Shader m_PS_Phong_DiffTexture_singleLight;
	ConstantBuffer m_phongMaterialCB;


	/*struct alignas(16) MatrixAl16
	{
		rfm::Matrix m;
	};*/

	

	//std::vector<RendUnitIDAndTransform> m_unitsToRender;
	std::vector<RendUnitIDAndTransform> m_colorUnits;
	std::vector<RendUnitIDAndTransform> m_diffTextureUnits;
};

