#include "pch.hpp"
#include "Renderer.h"
#include "rfEntity.hpp"
#include "RenderComponents.h"
#include "LowLvlGfx.h"
#include "AssetManager.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Render()
{
	for (const auto& m : rfe::EntityReg::getComponentArray<DiffuseTexturMaterialComp>())
	{
		auto diffuseTex = AssetManager::Get().GetTexture2D(m.textureID);
		if (auto ib = m.getEntity().getComponent<IndexedMeshComp>(); ib)
		{

		}
		
	}
}
