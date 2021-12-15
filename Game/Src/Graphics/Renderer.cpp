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
	

	
	m_sharedRenderResources->m_linearWrapSampler = LowLvlGfx::CreateSampler(standardSamplers::g_linear_wrap);

	SetUpHdrRTV();

	m_phongRenderer = PhongRenderer(m_sharedRenderResources->weak_from_this());
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
	m_phongRenderer.PreProcess(m_vp);
	m_phongRenderer.Render(m_vp);
}

void Renderer::SubmitToRender(rfe::Entity& camera)
{
	AssetManager& assetMan = AssetManager::Get();
	for (const auto& rendComp : rfe::EntityReg::getComponentArray<RenderModelComp>())
	{
		EntityID entID = rendComp.getEntityID();
		Transform worldMatrix = EntityReg::getComponent<TransformComp>(entID)->transform;
		if (rendComp.renderPass == RenderModelComp::RenderPassEnum::phong)
		{
			RenderUnitID b = rendComp.renderUnitBegin;
			RenderUnitID e = rendComp.renderUnitEnd;
			assert(b <= e);
			if (b | e)
			{
				for (RenderUnitID i = b; i < e; i++)
				{
					MaterialType matType = assetMan.GetRenderUnit(i).material.type;
					if ((matType & MaterialType::transparent) == MaterialType::transparent)
					{
						m_transparentRenderUnits.emplace_back(i, worldMatrix, matType);
					}
					else
					{
						m_phongRenderer.Submit(i, worldMatrix, matType);
					}
				}
			}
			else
			{
				MaterialType matType = assetMan.GetRenderUnit(rendComp.renderUnitID).material.type;
				if ((matType & MaterialType::transparent) == MaterialType::transparent)
				{
					m_transparentRenderUnits.emplace_back(rendComp.renderUnitID, worldMatrix, matType);
				}
				else
				{
					m_phongRenderer.Submit(rendComp.renderUnitID, worldMatrix, matType);
				}
			}
		}
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
