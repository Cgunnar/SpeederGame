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



	constexpr UINT irrCubeSideLength = 32;
	D3D11_TEXTURE2D_DESC irrDescCube = {};
	irrDescCube.Width = irrCubeSideLength;
	irrDescCube.Height = irrCubeSideLength;
	irrDescCube.MipLevels = 1;
	irrDescCube.ArraySize = 6;
	irrDescCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	irrDescCube.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	irrDescCube.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	irrDescCube.Usage = D3D11_USAGE_DEFAULT;
	irrDescCube.CPUAccessFlags = 0;
	irrDescCube.SampleDesc.Count = 1;
	irrDescCube.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC irrViewDesc = {};
	irrViewDesc.Format = irrDescCube.Format;
	irrViewDesc.TextureCube.MostDetailedMip = 0;
	irrViewDesc.TextureCube.MipLevels = -1;
	irrViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc = {};
	uDesc.Format = irrDescCube.Format;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uDesc.Texture2DArray.ArraySize = irrDescCube.ArraySize;
	uDesc.Texture2DArray.MipSlice = 0;
	uDesc.Texture2DArray.FirstArraySlice = 0;

	m_irradianceCubeMap = LowLvlGfx::CreateTexture2D(irrDescCube);
	LowLvlGfx::CreateSRV(m_irradianceCubeMap, &irrViewDesc);
	LowLvlGfx::CreateUAV(m_irradianceCubeMap, &uDesc);

	ConvoluteDiffuseCubeMap(cubeMap, m_irradianceCubeMap);



	constexpr int specCubeSideLength = 1024;
	D3D11_TEXTURE2D_DESC specDescCube = {};
	specDescCube.Width = specCubeSideLength;
	specDescCube.Height = specCubeSideLength;
	specDescCube.MipLevels = 5;
	specDescCube.ArraySize = 6;
	specDescCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	specDescCube.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	specDescCube.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	specDescCube.Usage = D3D11_USAGE_DEFAULT;
	specDescCube.CPUAccessFlags = 0;
	specDescCube.SampleDesc.Count = 1;
	specDescCube.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC specViewDesc = {};
	specViewDesc.Format = specDescCube.Format;
	specViewDesc.TextureCube.MostDetailedMip = 0;
	specViewDesc.TextureCube.MipLevels = -1;
	specViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

	

	m_specularCubeMap = LowLvlGfx::CreateTexture2D(specDescCube);
	LowLvlGfx::CreateSRV(m_specularCubeMap, &specViewDesc);

	ConvoluteSpecularCubeMap(cubeMap, m_specularCubeMap);
}

void EnvironmentMap::UpdateEnvMap(std::shared_ptr<Texture2D> cubeMap)
{
	ConvoluteDiffuseCubeMap(cubeMap, m_irradianceCubeMap);
	ConvoluteSpecularCubeMap(cubeMap, m_specularCubeMap);
}

std::shared_ptr<Texture2D> EnvironmentMap::GetIrradianceCubeMap()
{
	return m_irradianceCubeMap;
}

std::shared_ptr<Texture2D> EnvironmentMap::GetSpecularCubeMap()
{
	return m_specularCubeMap;
}


void EnvironmentMap::ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> irrMapOut)
{
	D3D11_TEXTURE2D_DESC outDesc;
	irrMapOut->buffer->GetDesc(&outDesc);
	LowLvlGfx::BindSRV(envMap, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::BindUAV(irrMapOut, 0);
	LowLvlGfx::Bind(s_convolute_DiffIrrCubeCS);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);
	assert(outDesc.Width == outDesc.Height);
	LowLvlGfx::Context()->Dispatch(outDesc.Width / 32, outDesc.Width / 32, 6);

	LowLvlGfx::BindUAVs({}); //unbind
}

void EnvironmentMap::ConvoluteSpecularCubeMap(std::shared_ptr<Texture2D> envMap, std::shared_ptr<Texture2D> specMapOut)
{
	D3D11_TEXTURE2D_DESC outDesc;
	specMapOut->buffer->GetDesc(&outDesc);
	assert(outDesc.Width == outDesc.Height);

	//copy skybox to output mipLevel 0
	D3D11_TEXTURE2D_DESC envDesc;
	envMap->buffer.Get()->GetDesc(&envDesc);
	for (int i = 0; i < 6; i++)
	{
		UINT destIndex = D3D11CalcSubresource(0, i, outDesc.MipLevels);
		UINT srcIndex = D3D11CalcSubresource(0, i, envDesc.MipLevels);
		LowLvlGfx::Context()->CopySubresourceRegion(specMapOut->buffer.Get(), destIndex, 0, 0, 0, envMap->buffer.Get(), srcIndex, nullptr);
	}

	LowLvlGfx::BindSRV(envMap, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::Bind(s_spmapCS);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);

	float deltaRoughness = 1.0f / (float)(outDesc.MipLevels - 1);
	rfm::Vector4 roughness;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uDesc = {};
	uDesc.Format = outDesc.Format;
	uDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uDesc.Texture2DArray.ArraySize = outDesc.ArraySize;
	uDesc.Texture2DArray.MipSlice = 0;
	uDesc.Texture2DArray.FirstArraySlice = 0;
	for (UINT i = 1; i < outDesc.MipLevels; i++)
	{
		uDesc.Texture2DArray.MipSlice = i;
		LowLvlGfx::CreateUAV(specMapOut, &uDesc);
		LowLvlGfx::BindUAV(specMapOut, 0);
		roughness.x = i * deltaRoughness;
		LowLvlGfx::UpdateBuffer(m_roughnessCB, &roughness);
		LowLvlGfx::Bind(m_roughnessCB, ShaderType::COMPUTESHADER, 5);

		UINT thrdGrSize = std::max(1u, outDesc.Width >> i);
		LowLvlGfx::Context()->Dispatch(thrdGrSize, thrdGrSize, 6);

	}
	LowLvlGfx::BindUAVs({}); //unbind
	LowLvlGfx::BindSRVs({}, ShaderType::COMPUTESHADER); //unbind
}