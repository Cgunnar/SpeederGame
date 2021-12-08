#include "pch.hpp"
#include "Renderer.h"
#include "rfEntity.hpp"
#include "RenderComponents.h"
#include "StandardComponents.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "RimfrostMath.hpp"

using namespace rfm;
using namespace rfe;



struct alignas(16) PhongMaterial
{
	Vector4 ka;
	Vector4 kd;
	Vector3 ks;
	float shininess;
};

Renderer::Renderer()
{
	m_vp.P = Matrix(PIDIV4, 16.0f / 9.0f, 0.01f, 1000.0f);

	m_worldMatrixCB = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	m_phongMaterialCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PhongMaterial), BufferDesc::USAGE::DYNAMIC });
	m_pointLightCB = LowLvlGfx::CreateConstantBuffer({ sizeof(PointLight), BufferDesc::USAGE::DYNAMIC });
	m_vpCB = LowLvlGfx::CreateConstantBuffer({ 2 * sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });


	m_vertexShader = LowLvlGfx::CreateShader("Src/Shaders/VertexShader.hlsl", ShaderType::VERTEXSHADER);
	m_phongPS = LowLvlGfx::CreateShader("Src/Shaders/PS_Phong_DiffTexture_singleLight.hlsl", ShaderType::PIXELSHADER);
	m_sampler = LowLvlGfx::CreateSampler(standardSamplers::g_linear_wrap);
}

Renderer::~Renderer()
{
}


void Renderer::Render(rfe::Entity& camera)
{
	auto& pointLights = rfe::EntityReg::getComponentArray<PointLightComp>();
	assert(!pointLights.empty());
	PointLight p = pointLights[0].pointLight;
	LowLvlGfx::UpdateBuffer(m_pointLightCB, &p);


	LowLvlGfx::SetViewPort(LowLvlGfx::GetResolution());
	m_vp.V = inverse(*camera.getComponent<TransformComp>());
	LowLvlGfx::UpdateBuffer(m_vpCB, &m_vp);

	//PhongRender(camera);
	RunRenderPasses(camera);
}

//void Renderer::PhongRender(rfe::Entity& camera)
//{
//	
//	LowLvlGfx::Bind(m_vpCB, ShaderType::VERTEXSHADER, 1);
//	LowLvlGfx::Bind(m_vpCB, ShaderType::PIXELSHADER, 1);
//
//	LowLvlGfx::Bind(m_vertexShader);
//	LowLvlGfx::Bind(m_phongPS);
//	LowLvlGfx::Bind(m_pointLightCB, ShaderType::PIXELSHADER, 0);
//	LowLvlGfx::Bind(m_sampler, ShaderType::PIXELSHADER, 0);
//	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	for (const auto& m : rfe::EntityReg::getComponentArray<DiffuseTexturMaterialComp>())
//	{
//		auto diffuseTex = AssetManager::Get().GetTexture2D(m.textureID);
//		if (auto ib = rfe::EntityReg::getComponent<IndexedMeshComp>(m.getEntityID()); ib)
//		{
//			PhongMaterial mat;
//			mat.ks = m.specularColor;
//			mat.shininess = m.shininess;
//			LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
//			LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);
//
//			LowLvlGfx::Bind(ib->vertexBuffer);
//			LowLvlGfx::Bind(ib->indexBuffer);
//			LowLvlGfx::UpdateBuffer(m_worldMatrixCB, &rfe::EntityReg::getComponent<TransformComp>(m.getEntityID())->transform);
//			LowLvlGfx::Bind(m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
//			LowLvlGfx::BindSRV(AssetManager::Get().GetTexture2D(m.textureID), ShaderType::PIXELSHADER, 0);
//			LowLvlGfx::DrawIndexed(ib->indexBuffer.GetIndexCount(), 0, 0);
//		}
//
//	}
//}

void Renderer::RunRenderPasses(rfe::Entity& camera)
{
	//sort RenderComp and then have switch case for renderpasses, sort more and instance



	//only one pass for now, fix loop for more passes later
	

	LowLvlGfx::Bind(m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::Bind(m_vpCB, ShaderType::PIXELSHADER, 1);

	LowLvlGfx::Bind(m_vertexShader);
	LowLvlGfx::Bind(m_phongPS);
	LowLvlGfx::Bind(m_pointLightCB, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(m_sampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	AssetManager& assetMan = AssetManager::Get();
	for (const auto& rendComp : rfe::EntityReg::getComponentArray<RenderComp>())
	{

		//it can render a range of RenderUnits, but need know what renderpass thay should belong to, add this to RenderUnit
		// and fix a function where RenderUnits are submitted insead of just renderd in this function
		/*for (RenderUnitID i = rendComp.renderUnitBegin; i < rendComp.renderUnitEnd; i++)
		{
			const RenderUnit& renderUnit = assetMan.GetRenderUnit(i);
		}*/



		EntityID entID = rendComp.getEntityID();
		const RenderUnit& renderUnit = assetMan.GetRenderUnit(rendComp.renderUnitID);
		auto worldMatrix = EntityReg::getComponent<TransformComp>(entID)->transform;

		auto diffTex = assetMan.GetTexture2D(renderUnit.material.diffuseTextureID);

		PhongMaterial mat;
		mat.ks = renderUnit.material.specularColor;
		mat.shininess = renderUnit.material.shininess;
		LowLvlGfx::UpdateBuffer(m_phongMaterialCB, &mat);
		LowLvlGfx::Bind(m_phongMaterialCB, ShaderType::PIXELSHADER, 2);

		LowLvlGfx::Bind(renderUnit.subMesh.vb);
		LowLvlGfx::Bind(renderUnit.subMesh.ib);
		LowLvlGfx::UpdateBuffer(m_worldMatrixCB, &worldMatrix);
		LowLvlGfx::Bind(m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);

		LowLvlGfx::BindSRV(diffTex, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::DrawIndexed(renderUnit.subMesh.indexCount, renderUnit.subMesh.startIndexLocation, renderUnit.subMesh.baseVertexLocation);

	}
}
