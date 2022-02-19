#pragma once

#include "GraphicsUtilityTypes.h"
#include "GraphicsResources.h"
namespace GfxHelpers
{
	uint32_t CalcMipNumber(uint32_t w, uint32_t h);

	std::shared_ptr<Texture2D> LoadImageToTex2D(const std::string& path, LoadTexFlag flags);
	void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride);

	std::shared_ptr<Texture2D> CreateEmptyCubeMap(uint32_t cubeSideLength, bool mipMapping);
}

