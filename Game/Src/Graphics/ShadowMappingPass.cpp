#include "pch.hpp"
#include "ShadowMappingPass.h"
#include "RenderComponents.h"
#include "LowLvlGfx.h"

using namespace rfm;

ShadowMappingPass::ShadowMappingPass(uint32_t res) : m_res(res)
{
	assert(LowLvlGfx::IsValid());

	m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VS_ShadowMapping_POS_NOR_UV.hlsl", ShaderType::VERTEXSHADER);
	m_emptyPixelShader = LowLvlGfx::CreateShader("Src/Shaders/PS_empty.hlsl", ShaderType::PIXELSHADER);

	m_viewProjCB = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });

	m_vp.P = OrthographicProjectionMatrix(100, 100, 0.01f, 1000);

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

	m_shadowMap = LowLvlGfx::CreateTexture2D(desc);
	LowLvlGfx::CreateSRV(m_shadowMap, &viewDesc);
	LowLvlGfx::CreateDSV(m_shadowMap, &depthDesc);

	
	
}

void ShadowMappingPass::DrawFromDirLight(rfm::Vector3 lightDirection, const std::vector<RendCompAndTransform>& geometyToRender)
{
	lightDirection.normalize();
	//m_vp.V = LookAt(-lightDirection * 200, 0);
	m_vp.V = LookAt({-2, 10, 0}, { 0,0,0 });

	LowLvlGfx::UpdateBuffer(m_viewProjCB, &m_vp);
	LowLvlGfx::Bind(m_viewProjCB, ShaderType::PIXELSHADER, 1);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	LowLvlGfx::SetViewPort({ m_res, m_res });
	LowLvlGfx::ClearDSV(m_shadowMap);
	LowLvlGfx::BindRTVs({}, m_shadowMap);
	LowLvlGfx::Bind(m_vertexShader);
	LowLvlGfx::Bind(m_emptyPixelShader);


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
				Draw(am.GetRenderUnit(i).subMesh, rt.worldMatrix);
			}
		}
		else
		{
			Draw(am.GetRenderUnit(rt.rendComp.renderUnitID).subMesh, rt.worldMatrix);
		}
	}




}

void ShadowMappingPass::Draw(const SubMesh& mesh, const rfm::Matrix& worldMatrix)
{
	LowLvlGfx::Bind(mesh.ib);
	LowLvlGfx::Bind(mesh.vb);
	LowLvlGfx::DrawIndexed(mesh.indexCount, mesh.startIndexLocation, mesh.baseVertexLocation);
}
