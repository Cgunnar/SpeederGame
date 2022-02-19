#include "pch.hpp"
#include "EnvironmentMap.h"
#include "LowLvlGfx.h"
#include "Renderer.h"

Shader EnvironmentMap::s_convolute_DiffIrrCubeCS;
Shader EnvironmentMap::s_spmapCS;

EnvironmentMap::EnvironmentMap()
{

}

EnvironmentMap::EnvironmentMap(std::shared_ptr<Texture2D> cubeMap)
{
	if(!s_convolute_DiffIrrCubeCS.IsValid()) s_convolute_DiffIrrCubeCS = LowLvlGfx::CreateShader("Src/Shaders/CS/CS_convolute_DiffIrrCube.hlsl", ShaderType::COMPUTESHADER);
	if(!s_spmapCS.IsValid()) s_spmapCS = LowLvlGfx::CreateShader("Src/Shaders/CS/spmap.hlsl", ShaderType::COMPUTESHADER);
	m_roughnessCB = LowLvlGfx::CreateConstantBuffer({ .size = 16, .usage = BufferDesc::USAGE::DYNAMIC });

	m_irradianceCubeMap = ConvoluteDiffuseCubeMap(cubeMap);
	m_specularCubeMap = ConvoluteSpecularCubeMap(cubeMap);
	/*m_irradianceCubeMap = irrMap;
	m_specularCubeMap = specMap;*/
}

std::shared_ptr<Texture2D> EnvironmentMap::GetIrradianceCubeMap()
{
	return m_irradianceCubeMap;
}

std::shared_ptr<Texture2D> EnvironmentMap::GetSpecularCubeMap()
{
	return m_specularCubeMap;
}


std::shared_ptr<Texture2D> EnvironmentMap::ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap)
{
	constexpr UINT cubeSideLength = 32;
	D3D11_TEXTURE2D_DESC descCube = {};
	descCube.Width = cubeSideLength;
	descCube.Height = cubeSideLength;
	descCube.MipLevels = 1;
	descCube.ArraySize = 6;
	descCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descCube.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descCube.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	descCube.Usage = D3D11_USAGE_DEFAULT;
	descCube.CPUAccessFlags = 0;
	descCube.SampleDesc.Count = 1;
	descCube.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Format = descCube.Format;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = -1;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc = {};
	uDesc.Format = descCube.Format;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uDesc.Texture2DArray.ArraySize = descCube.ArraySize;
	uDesc.Texture2DArray.MipSlice = 0;
	uDesc.Texture2DArray.FirstArraySlice = 0;

	std::shared_ptr<Texture2D> outPutCubeMap = LowLvlGfx::CreateTexture2D(descCube);
	LowLvlGfx::CreateSRV(outPutCubeMap, &viewDesc);
	LowLvlGfx::CreateUAV(outPutCubeMap, &uDesc);

	LowLvlGfx::BindSRV(envMap, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::BindUAV(outPutCubeMap, 0);
	LowLvlGfx::Bind(s_convolute_DiffIrrCubeCS);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);

	LowLvlGfx::Context()->Dispatch(cubeSideLength / 32, cubeSideLength / 32, 6);

	LowLvlGfx::BindUAVs({}); //unbind


	return outPutCubeMap;
}

std::shared_ptr<Texture2D> EnvironmentMap::ConvoluteSpecularCubeMap(std::shared_ptr<Texture2D> envMap)
{
	constexpr int cubeSideLength = 1024;
	D3D11_TEXTURE2D_DESC descCube = {};
	descCube.Width = cubeSideLength;
	descCube.Height = cubeSideLength;
	descCube.MipLevels = 5;
	descCube.ArraySize = 6;
	descCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descCube.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descCube.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	descCube.Usage = D3D11_USAGE_DEFAULT;
	descCube.CPUAccessFlags = 0;
	descCube.SampleDesc.Count = 1;
	descCube.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.Format = descCube.Format;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = -1;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc = {};
	uDesc.Format = descCube.Format;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uDesc.Texture2DArray.ArraySize = descCube.ArraySize;
	uDesc.Texture2DArray.MipSlice = 0;
	uDesc.Texture2DArray.FirstArraySlice = 0;

	std::shared_ptr<Texture2D> outPutCubeMap = LowLvlGfx::CreateTexture2D(descCube);
	LowLvlGfx::CreateSRV(outPutCubeMap, &viewDesc);

	//copy skybox to output mipLevel 0
	D3D11_TEXTURE2D_DESC envDesc;
	envMap->buffer.Get()->GetDesc(&envDesc);
	for (int i = 0; i < 6; i++)
	{
		UINT destIndex = D3D11CalcSubresource(0, i, descCube.MipLevels);
		UINT srcIndex = D3D11CalcSubresource(0, i, envDesc.MipLevels);
		LowLvlGfx::Context()->CopySubresourceRegion(outPutCubeMap->buffer.Get(), destIndex, 0, 0, 0, envMap->buffer.Get(), srcIndex, nullptr);
	}

	LowLvlGfx::BindSRV(envMap, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::Bind(s_spmapCS);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);

	float deltaRoughness = 1.0f / (float)(descCube.MipLevels - 1);
	rfm::Vector4 roughness;
	for (unsigned int i = 1; i < descCube.MipLevels; i++)
	{
		uDesc.Texture2DArray.MipSlice = i;
		LowLvlGfx::CreateUAV(outPutCubeMap, &uDesc);
		LowLvlGfx::BindUAV(outPutCubeMap, 0);
		roughness.x = i * deltaRoughness;
		LowLvlGfx::UpdateBuffer(m_roughnessCB, &roughness);
		LowLvlGfx::Bind(m_roughnessCB, ShaderType::COMPUTESHADER, 5);

		UINT thrdGrSize = std::max(1, cubeSideLength >> i);
		LowLvlGfx::Context()->Dispatch(thrdGrSize, thrdGrSize, 6);

	}
	LowLvlGfx::BindUAVs({}); //unbind
	LowLvlGfx::BindSRVs({}, ShaderType::COMPUTESHADER); //unbind

	return outPutCubeMap;
}