#include "pch.hpp"
#include "ShadowMappingPass.h"
#include "RenderComponents.h"
#include "LowLvlGfx.h"
#include "Renderer.h"

using namespace rfm;


ShadowMappingPass::ShadowMappingPass(std::weak_ptr<SharedRenderResources> sharedRes, uint32_t res) : m_res(res), m_sharedRenderResources(sharedRes)
{
	m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VS_ShadowMapping.hlsl", ShaderType::VERTEXSHADER);
	m_emptyPixelShader = LowLvlGfx::CreateShader("Src/Shaders/PS_empty.hlsl", ShaderType::PIXELSHADER);

	m_projectionMatrix = OrthographicProjectionMatrix(32, 32, 0.01f, 60);

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = res;
	desc.Height = res;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = 1;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc{};
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthDesc.Texture2D.MipSlice = 0;

	auto rendRes = m_sharedRenderResources.lock();
	rendRes->shadowMap = LowLvlGfx::CreateTexture2D(desc);
	LowLvlGfx::CreateSRV(rendRes->shadowMap, &viewDesc);
	LowLvlGfx::CreateDSV(rendRes->shadowMap, &depthDesc);
}

void ShadowMappingPass::DrawFromDirLight(rfe::Entity camera, rfm::Vector3 lightDirection, const std::vector<RendCompAndTransform>& geometyToRender)
{
	Vector3 cameraPos = camera.GetComponent<TransformComp>()->transform.getTranslation();
	m_projectionMatrix = OrthographicProjectionMatrix(64, 64, 0.01f, 500);

	auto rendRes = m_sharedRenderResources.lock();

	lightDirection.normalize();
	m_vp = m_projectionMatrix * LookAt(cameraPos - lightDirection * 50, cameraPos);

	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	LowLvlGfx::SetViewPort({ m_res, m_res });
	LowLvlGfx::ClearDSV(rendRes->shadowMap);
	LowLvlGfx::BindRTVs({}, rendRes->shadowMap);
	LowLvlGfx::Bind(m_vertexShader);
	LowLvlGfx::Bind(m_emptyPixelShader);
	LowLvlGfx::Bind(rendRes->m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);

	AssetManager& am = AssetManager::Get();
	for (const auto & rt : geometyToRender)
	{
		RenderUnitID b = rt.rendComp.renderUnitBegin;
		RenderUnitID e = rt.rendComp.renderUnitEnd;
		assert(b <= e);
		if (b | e)
		{
			for (RenderUnitID i = b; i < e; i++)
			{
				const SubMesh& mesh = am.GetRenderUnit(i).subMesh;
				Matrix mvp = m_vp * rt.worldMatrix;
				LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &mvp);
				LowLvlGfx::Bind(mesh.ib);
				LowLvlGfx::Bind(mesh.vb);
				LowLvlGfx::DrawIndexed(mesh.indexCount, mesh.startIndexLocation, mesh.baseVertexLocation);
			}
		}
		else
		{
			const SubMesh& mesh = am.GetRenderUnit(rt.rendComp.renderUnitID).subMesh;
			Matrix mvp = m_vp * rt.worldMatrix;
			LowLvlGfx::UpdateBuffer(rendRes->m_worldMatrixCB, &mvp);
			LowLvlGfx::Bind(mesh.ib);
			LowLvlGfx::Bind(mesh.vb);
			LowLvlGfx::DrawIndexed(mesh.indexCount, mesh.startIndexLocation, mesh.baseVertexLocation);
		}
	}
}

const Matrix* ShadowMappingPass::GetViewProjectionMatrix() const
{
	return &m_vp;
}
