#include "pch.hpp"
#include "Renderer.h"
#include "rfEntity.hpp"
#include "RenderComponents.h"
#include "StandardComponents.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"
#include "RimfrostMath.hpp"

using namespace rfm;
Transform worldMatrix;
Renderer::Renderer()
{
	m_worldMatrixCBuffer = LowLvlGfx::CreateConstantBuffer({ sizeof(Matrix), BufferDesc::USAGE::DYNAMIC });
	worldMatrix.setTranslation(0, 3, 0);
}

Renderer::~Renderer()
{
}


void Renderer::Render()
{

	/*LowLvlGfx::Bind(Quad::vertexBuffer);
		LowLvlGfx::Bind(Quad::indexBuffer);
		LowLvlGfx::UpdateBuffer(worldMatrixCBuffer, &myBox.worldMatrix);
		LowLvlGfx::Bind(worldMatrixCBuffer, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::BindSRV(myTexture, ShaderType::PIXELSHADER, 0);
		LowLvlGfx::DrawIndexed(Quad::indexBuffer.GetIndexCount(), 0, 0);*/
	for (const auto& m : rfe::EntityReg::getComponentArray<DiffuseTexturMaterialComp>())
	{
		auto diffuseTex = AssetManager::Get().GetTexture2D(m.textureID);
		if (auto ib = rfe::EntityReg::getComponent<IndexedMeshComp>(m.getEntityID()); ib)
		{
			LowLvlGfx::Bind(ib->vertexBuffer);
			LowLvlGfx::Bind(ib->indexBuffer);

			//FIX
			LowLvlGfx::UpdateBuffer(m_worldMatrixCBuffer, &worldMatrix); // this is tempglobla matrix
			//LowLvlGfx::UpdateBuffer(m_worldMatrixCBuffer, rfe::EntityReg::getComponent<TransformComp>(m.getEntityID())); //FIX
			
			LowLvlGfx::Bind(m_worldMatrixCBuffer, ShaderType::VERTEXSHADER, 0);
			LowLvlGfx::BindSRV(AssetManager::Get().GetTexture2D(m.textureID), ShaderType::PIXELSHADER, 0);
			LowLvlGfx::DrawIndexed(ib->indexBuffer.GetIndexCount(), 0, 0);
		}
		
	}
}
