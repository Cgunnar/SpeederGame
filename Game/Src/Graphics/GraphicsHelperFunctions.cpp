#include "pch.hpp"
#include "GraphicsHelperFunctions.h"
#include "LowLvlGfx.h"
#include "ReadImg.hpp"


namespace GfxHelpers
{
	uint32_t CalcMipNumber(uint32_t w, uint32_t h)
	{
		return 1 + static_cast<int>(floor(log2(std::max(w, h))));
	}
	
	void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride)
	{
		assert(dataPtr);

		subResMipArray = new D3D11_SUBRESOURCE_DATA[mipNumber];
		int SysMemPitch = stride;

		for (int i = 0; i < mipNumber; i++)
		{
			subResMipArray[i].pSysMem = dataPtr;
			subResMipArray[i].SysMemPitch = SysMemPitch;
			subResMipArray[i].SysMemSlicePitch = 0;
			SysMemPitch >>= 1;
		}
	}

	std::shared_ptr<Texture2D> LoadImageToTex2D(const std::string& path, LoadTexFlag flags)
	{
		bool srgb = (flags & LoadTexFlag::LinearColorSpace) == 0;
		bool generateMips = (flags & LoadTexFlag::GenerateMips) != 0;

		MyImageStruct im;
		readImage(im, path);
		D3D11_TEXTURE2D_DESC desc;
		desc.CPUAccessFlags = 0;

		desc.Height = im.height;
		desc.Width = im.width;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

		if (generateMips)
		{
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			desc.MipLevels = im.mipNumber;
		}
		else
		{
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.MiscFlags = 0;
			desc.MipLevels = 1;
		}

		std::shared_ptr<Texture2D> myTexture;

		if (generateMips)
		{
			D3D11_SUBRESOURCE_DATA* subResMipArray = nullptr;
			SetSubResDataMips(im.imagePtr, subResMipArray, im.mipNumber, im.stride);
			myTexture = LowLvlGfx::CreateTexture2D(desc, subResMipArray);
			delete[] subResMipArray;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;
			LowLvlGfx::CreateSRV(myTexture, &srvDesc);
		}
		else
		{
			D3D11_SUBRESOURCE_DATA subRes;
			subRes.pSysMem = im.imagePtr;
			subRes.SysMemPitch = im.stride;
			subRes.SysMemSlicePitch = 0;
			myTexture = LowLvlGfx::CreateTexture2D(desc, &subRes);
			LowLvlGfx::CreateSRV(myTexture, nullptr);
		}
		return myTexture;
	}



	std::shared_ptr<Texture2D> CreateEmptyCubeMap(uint32_t cubeSideLength, bool mipMapping)
	{
		D3D11_TEXTURE2D_DESC descCube = {};
		descCube.Width = cubeSideLength;
		descCube.Height = cubeSideLength;
		descCube.MipLevels = mipMapping ? GfxHelpers::CalcMipNumber(cubeSideLength, cubeSideLength) : 1;
		descCube.ArraySize = 6;
		descCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		descCube.BindFlags = mipMapping ? D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET : D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		descCube.MiscFlags = mipMapping ? D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS : D3D11_RESOURCE_MISC_TEXTURECUBE;
		descCube.Usage = D3D11_USAGE_DEFAULT;
		descCube.CPUAccessFlags = 0;
		descCube.SampleDesc.Count = 1;
		descCube.SampleDesc.Quality = 0;

		std::shared_ptr<Texture2D> cubeMap = LowLvlGfx::CreateTexture2D(descCube);

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = descCube.Format;
		viewDesc.TextureCube.MostDetailedMip = 0;
		viewDesc.TextureCube.MipLevels = -1;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

		LowLvlGfx::CreateSRV(cubeMap, &viewDesc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc = {};
		uDesc.Format = descCube.Format;
		uDesc.Texture2DArray.ArraySize = descCube.ArraySize;
		uDesc.Texture2DArray.FirstArraySlice = 0;
		uDesc.Texture2DArray.MipSlice = 0;
		uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;

		LowLvlGfx::CreateUAV(cubeMap, &uDesc);
		return cubeMap;
	}
}
