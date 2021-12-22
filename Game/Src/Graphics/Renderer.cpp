#include "pch.hpp"
#include "Renderer.h"
#include "rfEntity.hpp"
#include "RenderComponents.h"
#include "StandardComponents.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "RimfrostMath.hpp"
#include "Material.h"

using namespace rfm;
using namespace rfe;




Renderer::Renderer()
{
	m_vp.P = Matrix(PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);
	m_sharedRenderResources = std::make_shared<SharedRenderResources>();

	m_sharedRenderResources->m_worldMatrixCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	m_sharedRenderResources->m_vpCB = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });

	m_sharedRenderResources->m_pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PointLight), BufferDesc::USAGE::DYNAMIC });


	m_sharedRenderResources->m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	m_sharedRenderResources->m_vertexShaderNormalMap = LowLvlGfx::CreateShader("Src/Shaders/VS_NormalMap.hlsl", ShaderType::VERTEXSHADER);



	m_sharedRenderResources->m_linearWrapSampler = LowLvlGfx::Create(standardDescriptors::g_linear_wrap);

	SetUpHdrRTV();

	m_phongRenderer = PhongRenderer(m_sharedRenderResources->weak_from_this());
	m_pbrRenderer = PbrRenderer(m_sharedRenderResources->weak_from_this());
}

Renderer::~Renderer()
{
}


void Renderer::Render(rfe::Entity& camera)
{
	auto& pointLights = rfe::EntityReg::getComponentArray<PointLightComp>();
	assert(!pointLights.empty());
	PointLight p = pointLights[0].pointLight;
	LowLvlGfx::UpdateBuffer(m_sharedRenderResources->m_pointLightCB, &p);


	LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());
	m_vp.V = inverse(*camera.getComponent<TransformComp>());
	LowLvlGfx::UpdateBuffer(m_sharedRenderResources->m_vpCB, &m_vp);

	SubmitToRender(camera);

	RenderAllPasses(m_vp, camera);

	/*m_phongRenderer.PreProcess(m_vp, camera, RenderFlag::none);
	m_phongRenderer.Render(m_vp, camera, RenderFlag::none);

	m_pbrRenderer.PreProcess(m_vp, camera, RenderFlag::none);
	m_pbrRenderer.Render(m_vp, camera, RenderFlag::none);*/

	SubmitAndRenderTransparentToInternalRenderers(m_vp, camera);

}



void Renderer::SubmitToRender(rfe::Entity& camera)
{
	AssetManager& assetMan = AssetManager::Get();
	for (const auto& rendComp : rfe::EntityReg::getComponentArray<RenderModelComp>())
	{
		EntityID entID = rendComp.getEntityID();
		Transform worldMatrix = EntityReg::getComponent<TransformComp>(entID)->transform;

		RenderUnitID b = rendComp.renderUnitBegin;
		RenderUnitID e = rendComp.renderUnitEnd;
		assert(b <= e);
		if (b | e)
		{
			for (RenderUnitID i = b; i < e; i++)
			{
				SubmitToInternalRenderers(assetMan, rendComp.renderPass, i, worldMatrix);
			}
		}
		else
		{
			SubmitToInternalRenderers(assetMan, rendComp.renderPass, rendComp.renderUnitID, worldMatrix);
		}
	}
}

void Renderer::SubmitToInternalRenderers(AssetManager& am, RenderPassEnum renderPass, RenderUnitID unitID, const rfm::Transform& worldMatrix)
{
	auto ru = am.GetRenderUnit(unitID);

	if ((ru.material.renderFlag & RenderFlag::alphaBlend) != 0)
	{
		m_transparentRenderUnits.push_back({ ru.material.renderFlag, renderPass, RendUnitIDAndTransform(unitID, worldMatrix, ru.material.type) });
	}
	else
	{
		m_renderPassesFlagged[ru.material.renderFlag].emplace_back(unitID, worldMatrix, ru.material.type);
	}
}


void Renderer::SubmitAndRenderTransparentToInternalRenderers(const VP& viewAndProjMatrix, rfe::Entity& camera)
{
	if (m_transparentRenderUnits.empty()) return;

	auto camTransform = camera.getComponent<TransformComp>()->transform;
	Vector3 forward = camTransform.forward();
	auto&& backToFront = [&forward](PassForTransparentUnits a, PassForTransparentUnits b)
	{
		const auto& posA = a.unit.worldMatrix.getTranslation();
		const auto& posB = b.unit.worldMatrix.getTranslation();
		return dot(posA, forward) < dot(posB, forward);
	};
	std::sort(m_transparentRenderUnits.begin(), m_transparentRenderUnits.end(), backToFront);

	MaterialType previusType = m_transparentRenderUnits[0].unit.type;
	RenderFlag previusRenderFlag = m_transparentRenderUnits[0].rendFlag;

	for (int i = 0; i < m_transparentRenderUnits.size(); i++)
	{
		auto& traUnit = m_transparentRenderUnits[i];
		if (traUnit.unit.type != previusType || previusRenderFlag != traUnit.rendFlag)
		{
			m_phongRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);
			m_pbrRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);
		}
		m_phongRenderer.Submit(traUnit.unit.id, traUnit.unit.worldMatrix, traUnit.unit.type);
		m_pbrRenderer.Submit(traUnit.unit.id, traUnit.unit.worldMatrix, traUnit.unit.type);
		previusType = traUnit.unit.type;
		previusRenderFlag = traUnit.rendFlag;
	}
	m_phongRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);
	m_pbrRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);

	m_transparentRenderUnits.clear();
}


void Renderer::RenderAllPasses(const VP& viewAndProjMatrix, rfe::Entity& camera)
{
	for (auto& it : m_renderPassesFlagged)
	{
		if (it.second.empty()) continue;
		for(auto& unit : it.second)
		{
			m_phongRenderer.Submit(unit.id, unit.worldMatrix, unit.type);
			m_pbrRenderer.Submit(unit.id, unit.worldMatrix, unit.type);
		}
		it.second.clear();
		m_phongRenderer.Render(viewAndProjMatrix, camera, it.first);
		m_pbrRenderer.Render(viewAndProjMatrix, camera, it.first);
	}
}

void Renderer::SetUpHdrRTV()
{
	Resolution res = LowLvlGfx::GetResolution();

	D3D11_TEXTURE2D_DESC desc2d;
	desc2d.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc2d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc2d.Usage = D3D11_USAGE_DEFAULT;
	desc2d.CPUAccessFlags = 0;
	desc2d.MiscFlags = 0;
	desc2d.SampleDesc.Count = 1;
	desc2d.SampleDesc.Quality = 0;
	desc2d.ArraySize = 1;
	desc2d.Width = res.width;
	desc2d.Height = res.height;
	desc2d.MipLevels = 0;

	m_sharedRenderResources->m_hdrRenderTarget = LowLvlGfx::CreateTexture2D(desc2d, nullptr, false);

	LowLvlGfx::CreateSRV(m_sharedRenderResources->m_hdrRenderTarget);

	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.Texture2D.MipSlice = 0;
	LowLvlGfx::CreateRTV(m_sharedRenderResources->m_hdrRenderTarget);


}
