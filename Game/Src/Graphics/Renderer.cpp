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

std::shared_ptr<SharedRenderResources> Renderer::s_sharedRenderResources = nullptr;

//Sprite testSprite;
Renderer::Renderer()
{
	m_vp.P = PerspectiveProjectionMatrix(PIDIV4, 16.0f / 9.0f, m_nearPlane, m_farPlane);
	s_sharedRenderResources = std::make_shared<SharedRenderResources>();

	s_sharedRenderResources->m_worldMatrixCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	s_sharedRenderResources->m_shadowMapViewProjCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	s_sharedRenderResources->m_vpCB = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });

	s_sharedRenderResources->m_pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PointLight), BufferDesc::USAGE::DYNAMIC });
	s_sharedRenderResources->m_dirLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(DirectionalLight), BufferDesc::USAGE::DYNAMIC });


	s_sharedRenderResources->m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	s_sharedRenderResources->m_vertexShaderNormalMap = LowLvlGfx::CreateShader("Src/Shaders/VS_NormalMap.hlsl", ShaderType::VERTEXSHADER);



	s_sharedRenderResources->m_linearWrapSampler = LowLvlGfx::Create(standardDescriptors::g_sample_linear_wrap);

	SetUpHdrRTV();

	m_spriteRenderer.Init();
	//GID texID = AssetManager::Get().LoadTex2DFromFile("Assets/testImg.png", LoadTexFlag::none);
	/*GID texID = AssetManager::Get().LoadTex2DFromFile("Assets/Hej.png", LoadTexFlag::none);
	testSprite = Sprite(AssetManager::Get().GetTexture2D(texID), { 0.5, 0.5 }, { 0.1,0.1 });*/
	m_pbrRenderer = PbrRenderer(s_sharedRenderResources->weak_from_this());
	m_shadowPass = ShadowMappingPass(s_sharedRenderResources->weak_from_this(), 8192 / 2);
}

Renderer::~Renderer()
{
	s_sharedRenderResources.reset();
	s_sharedRenderResources = nullptr;
}


void Renderer::RenderBegin(rfe::Entity& camera)
{
	LowLvlGfx::ClearRTV(0.1f, 0.2f, 0.4f, 0.0f, LowLvlGfx::GetBackBuffer());
	LowLvlGfx::ClearDSV(LowLvlGfx::GetDepthBuffer());
	Resolution res = LowLvlGfx::GetResolution();
	LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());
	m_vp.V = inverse(*camera.GetComponent<TransformComp>());
	m_vp.P = PerspectiveProjectionMatrix(PIDIV4, static_cast<float>(res.width) / static_cast<float>(res.height), m_nearPlane, m_farPlane);
	LowLvlGfx::UpdateBuffer(s_sharedRenderResources->m_vpCB, &m_vp);
}

void Renderer::RenderSkyBox(SkyBox& sky)
{
	if (sky.Ldr() != sky.Hdr())
	{
		LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());
		sky.Bind(*s_sharedRenderResources);
		LowLvlGfx::Context()->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		LowLvlGfx::Context()->IASetInputLayout(nullptr);
		LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		LowLvlGfx::Context()->Draw(36, 0);

		if (sky.m_irradianceCubeMap)
		{
			m_pbrRenderer.SetDiffuseIrradianceCubeMap(sky.m_irradianceCubeMap);
		}
		if (sky.m_specularCubeMap)
		{
			m_pbrRenderer.SetSpecularCubeMap(sky.m_specularCubeMap);
		}
		if (sky.m_splitSumMap)
		{
			m_pbrRenderer.SetSplitSumAproxLookUpMap(sky.m_splitSumMap);
		}
	}
}

void Renderer::Render(rfe::Entity& camera, DirectionalLight dirLight)
{
	LowLvlGfx::UnBindRasterizer(); // this will use the default rasterizer

	auto& pointLights = rfe::EntityReg::GetComponentArray<PointLightComp>();
	assert(!pointLights.empty());
	PointLight p = pointLights[0].pointLight;
	LowLvlGfx::UpdateBuffer(s_sharedRenderResources->m_pointLightCB, &p);

	LowLvlGfx::UpdateBuffer(s_sharedRenderResources->m_dirLightCB, &dirLight);

	CopyFromECS();
	SubmitToRender();

	//shadowMapping
	m_shadowPass.Bind(camera, dirLight.dir);
	for (auto& [flag, units] : m_renderPassesFlagged)
	{
		if (units.empty()) continue;
		if ((flag & RenderFlag::wireframe) != 0) continue;
		m_shadowPass.DrawFromDirLight(units);
	}
	

	LowLvlGfx::UpdateBuffer(s_sharedRenderResources->m_shadowMapViewProjCB, m_shadowPass.GetViewProjectionMatrix());

	//main rendering
	LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }, LowLvlGfx::GetDepthBuffer());
	LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());

	RenderAllPasses(m_vp, camera);
	SubmitAndRenderTransparentToInternalRenderers(m_vp, camera);



	//sprite rendering
	//m_spriteRenderer.Draw({ testSprite });
	
}

SharedRenderResources& Renderer::GetSharedRenderResources()
{
	assert(s_sharedRenderResources);
	return *s_sharedRenderResources;
}



void Renderer::CopyFromECS()
{
	const auto& rendCompArray = rfe::EntityReg::GetComponentArray<RenderModelComp>();
	m_rendCompAndTransformFromECS.clear();
	m_rendCompAndTransformFromECS.reserve(rendCompArray.size());
	for (const auto& rendComp : rendCompArray)
	{
		if (rendComp.visible)
		{
			EntityID entID = rendComp.GetEntityID();
			assert(EntityReg::GetComponent<TransformComp>(entID));
			m_rendCompAndTransformFromECS.emplace_back(
				EntityReg::GetComponent<TransformComp>(entID)->transform, rendComp.renderPass,
				rendComp.renderUnitID, rendComp.renderUnitBegin, rendComp.renderUnitEnd);
		}
	}

	const auto& rendUnitCompArray = rfe::EntityReg::GetComponentArray<RenderUnitComp>();
	for (const auto& rendComp : rendUnitCompArray)
	{
		if (rendComp.visible)
		{
			EntityID entID = rendComp.GetEntityID();
			assert(EntityReg::GetComponent<TransformComp>(entID));
			m_rendCompAndTransformFromECS.emplace_back(
				EntityReg::GetComponent<TransformComp>(entID)->transform,
				rendComp.renderPass, rendComp.unitID);
		}
	}
}

void Renderer::SubmitToRender()
{
	AssetManager& assetMan = AssetManager::Get();
	for (const auto& rt : m_rendCompAndTransformFromECS)
	{
		assert(rt.begin <= rt.end);
		if (rt.begin | rt.end)
		{
			for (RenderUnitID i = rt.begin; i < rt.end; i++)
			{
				SubmitToInternalRenderers(assetMan, rt.renderPass, i, rt.worldMatrix);
			}
		}
		else
		{
			SubmitToInternalRenderers(assetMan, rt.renderPass, rt.id, rt.worldMatrix);
		}
	}
}

void Renderer::SubmitToInternalRenderers(AssetManager& am, RenderPassEnum renderPass, RenderUnitID unitID, const rfm::Transform& worldMatrix)
{
	auto ru = am.GetRenderUnit(unitID);

	if ((ru.material.flags & RenderFlag::alphaBlend) != 0)
	{
		m_transparentRenderUnits.push_back({ ru.material.flags, renderPass, RendUnitIDAndTransform(unitID, worldMatrix, ru.material.GetType()) });
	}
	else
	{
		m_renderPassesFlagged[ru.material.flags].emplace_back(unitID, worldMatrix, ru.material.GetType());
	}
}


void Renderer::SubmitAndRenderTransparentToInternalRenderers(const VP& viewAndProjMatrix, rfe::Entity& camera)
{
	if (m_transparentRenderUnits.empty()) return;

	auto camTransform = camera.GetComponent<TransformComp>()->transform;
	Vector3 forward = camTransform.forward();
	auto&& backToFront = [&forward](PassForTransparentUnits a, PassForTransparentUnits b)
	{
		const auto& posA = a.unit.worldMatrix.getTranslation();
		const auto& posB = b.unit.worldMatrix.getTranslation();
		return dot(posA, forward) < dot(posB, forward);
	};
	std::sort(m_transparentRenderUnits.begin(), m_transparentRenderUnits.end(), backToFront);

	MaterialVariantEnum previusType = m_transparentRenderUnits[0].unit.type;
	RenderFlag previusRenderFlag = m_transparentRenderUnits[0].rendFlag;

	for (int i = 0; i < m_transparentRenderUnits.size(); i++)
	{
		auto& traUnit = m_transparentRenderUnits[i];
		if (traUnit.unit.type != previusType || previusRenderFlag != traUnit.rendFlag)
		{
			m_pbrRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);
		}
		m_pbrRenderer.Submit(traUnit.unit.id, traUnit.unit.worldMatrix, traUnit.unit.type);
		previusType = traUnit.unit.type;
		previusRenderFlag = traUnit.rendFlag;
	}
	m_pbrRenderer.Render(viewAndProjMatrix, camera, previusRenderFlag);

	m_transparentRenderUnits.clear();
}


void Renderer::RenderAllPasses(const VP& viewAndProjMatrix, rfe::Entity& camera)
{
	for (auto& [flag, units] : m_renderPassesFlagged)
	{
		if (units.empty()) continue;
		for (auto& unit : units)
		{
			m_pbrRenderer.Submit(unit.id, unit.worldMatrix, unit.type);
		}
		units.clear();
		m_pbrRenderer.Render(viewAndProjMatrix, camera, flag);
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

	s_sharedRenderResources->m_hdrRenderTarget = LowLvlGfx::CreateTexture2D(desc2d, nullptr, false);

	LowLvlGfx::CreateSRV(s_sharedRenderResources->m_hdrRenderTarget);

	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.Texture2D.MipSlice = 0;
	LowLvlGfx::CreateRTV(s_sharedRenderResources->m_hdrRenderTarget);
}
