#include "pch.hpp"
#include "SkyBox.h"
#include "ReadImg.hpp"
#include "LowLvlGfx.h"
#include <stb_image.h>
#include "Renderer.h"
#include "GraphicsHelperFunctions.h"


std::shared_ptr<Texture2D> LoadHdrTexture(const std::string& path);


void SkyBox::Init(const std::string& path, const std::string& irradianceMapPath)
{
	if (path.substr(path.length() - 4, 4) == ".hdr")
		InitCubeMapHDR(path, irradianceMapPath);
	else
		InitCubeMapLDR(path);

	m_skyBoxVS = LowLvlGfx::CreateShader("Src/Shaders/VS_SkyBox.hlsl", ShaderType::VERTEXSHADER);
	


	D3D11_RASTERIZER_DESC rzDesc = {};
	rzDesc.CullMode = D3D11_CULL_BACK;
	rzDesc.FillMode = D3D11_FILL_SOLID;
	rzDesc.FrontCounterClockwise = true;
	m_rasterizer = LowLvlGfx::Create(rzDesc);

	D3D11_SAMPLER_DESC sampDesc = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f,
		1,
		D3D11_COMPARISON_LESS_EQUAL,
		{0,0,0,0},
		0,
		D3D11_FLOAT32_MAX
	};
	
	m_sampler = LowLvlGfx::Create(sampDesc);

}

bool SkyBox::Hdr() const
{
	return m_hdr;
}

bool SkyBox::Ldr() const
{
	return m_ldr;
}

void SkyBox::Bind(SharedRenderResources& rendRes)
{
	assert(m_hdr || m_ldr);
	//if (m_ldr)
	//{
		LowLvlGfx::UpdateBuffer(rendRes.m_worldMatrixCB, &m_rotation);
		LowLvlGfx::Bind(rendRes.m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendRes.m_vpCB, ShaderType::VERTEXSHADER, 1);
		LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }); // this should unbind the z-buffer
		LowLvlGfx::Bind(m_skyBoxVS);
		LowLvlGfx::Bind(m_skyBoxPS);
		LowLvlGfx::BindSRV(m_skyBoxCubeMap, ShaderType::PIXELSHADER, 4);
		LowLvlGfx::Bind(m_sampler, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::Bind(m_rasterizer);
	//}
}

void SkyBox::SetRotation(rfm::Matrix rot)
{
	m_rotation = rot;
}

void SkyBox::InitCubeMapLDR(const std::string& path)
{
	assert(!m_ldr && !m_hdr);
	m_ldr = true;

	m_skyBoxPS = LowLvlGfx::CreateShader("Src/Shaders/PS_SkyBox.hlsl", ShaderType::PIXELSHADER);

	MyImageStruct skyFacesData[6]{};
	readImage(skyFacesData[0], path + "/posx.jpg");
	readImage(skyFacesData[1], path + "/negx.jpg");
	readImage(skyFacesData[2], path + "/posy.jpg");
	readImage(skyFacesData[3], path + "/negy.jpg");
	readImage(skyFacesData[4], path + "/posz.jpg");
	readImage(skyFacesData[5], path + "/negz.jpg");

	D3D11_TEXTURE2D_DESC desc = { };
	desc.Width = skyFacesData[0].width;
	desc.Height = skyFacesData[0].height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;


	D3D11_SUBRESOURCE_DATA subres[6] = { };
	for (int i = 0; i < 6; ++i)
	{
		subres[i].pSysMem = skyFacesData[i].imagePtr;
		subres[i].SysMemPitch = skyFacesData[i].stride;
		subres[i].SysMemSlicePitch = 0;
	}

	m_skyBoxCubeMap = LowLvlGfx::CreateTexture2D(desc, subres);

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = desc.Format;
	viewDesc.TextureCube.MipLevels = 1;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

	LowLvlGfx::CreateSRV(m_skyBoxCubeMap, &viewDesc);

	
}

void SkyBox::InitCubeMapHDR(const std::string& path, const std::string& irradianceMapPath)
{
	assert(std::filesystem::exists(path));
	assert(!m_ldr && !m_hdr);
	m_hdr = true;

	m_skyBoxPS = LowLvlGfx::CreateShader("Src/Shaders/PS_SkyBox_toneMapped.hlsl", ShaderType::PIXELSHADER);
	m_convolute_DiffIrrCubeCS = LowLvlGfx::CreateShader("Src/Shaders/CS/CS_convolute_DiffIrrCube.hlsl", ShaderType::COMPUTESHADER);
	m_skyBoxCubeMap = LoadEquirectangularMapToCubeMap(path);

	if (!irradianceMapPath.empty())
	{
		assert(irradianceMapPath.substr(irradianceMapPath.length() - 4, 4) == ".hdr");

		m_irradianceCubeMap = LoadEquirectangularMapToCubeMap(irradianceMapPath, 32);
	}
	else
	{
		m_irradianceCubeMap = ConvoluteDiffuseCubeMap(m_skyBoxCubeMap);
	}
	
}

std::shared_ptr<Texture2D> SkyBox::ConvoluteDiffuseCubeMap(std::shared_ptr<Texture2D> envMap)
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
	LowLvlGfx::Bind(m_convolute_DiffIrrCubeCS);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);

	LowLvlGfx::Context()->Dispatch(1, 1, 6);

	LowLvlGfx::BindUAVs(); //unbind


	return outPutCubeMap;
}

std::shared_ptr<Texture2D> LoadHdrTexture(const std::string& path)
{
	int w, h, c;
	float* hdrImage = stbi_loadf(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
	assert(hdrImage);


	D3D11_TEXTURE2D_DESC descEq = {};
	descEq.Width = w;
	descEq.Height = h;
	descEq.MipLevels = 1;
	descEq.ArraySize = 1;
	descEq.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descEq.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	descEq.Usage = D3D11_USAGE_IMMUTABLE;
	descEq.MiscFlags = 0;
	descEq.CPUAccessFlags = 0;
	descEq.SampleDesc.Count = 1;
	descEq.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA subEq;
	subEq.pSysMem = hdrImage;
	subEq.SysMemPitch = w * 16; // rgba float
	subEq.SysMemSlicePitch = 0;
	std::shared_ptr<Texture2D> equirectTex = LowLvlGfx::CreateTexture2D(descEq, &subEq);
	LowLvlGfx::CreateSRV(equirectTex);

	stbi_image_free(hdrImage);

	return equirectTex;
}

std::shared_ptr<Texture2D> SkyBox::LoadEquirectangularMapToCubeMap(const std::string& path, uint32_t cubeSideLength)
{
	std::shared_ptr<Texture2D> equirectTex = LoadHdrTexture(path);

	D3D11_TEXTURE2D_DESC descCube = {};
	descCube.Width = cubeSideLength;
	descCube.Height = cubeSideLength;
	descCube.MipLevels = 1;// GfxHelpers::CalcMipNumber(cubeSideLength, cubeSideLength);
	descCube.ArraySize = 6;
	descCube.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descCube.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;// | D3D11_BIND_RENDER_TARGET;
	descCube.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;// | D3D11_RESOURCE_MISC_GENERATE_MIPS;
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

	m_eq2cubeCS = LowLvlGfx::CreateShader("Src/Shaders/CS_equirect2cube.hlsl", ShaderType::COMPUTESHADER);

	LowLvlGfx::BindUAVs({ cubeMap });
	LowLvlGfx::BindSRV(equirectTex, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::Bind(Renderer::GetSharedRenderResources().m_linearWrapSampler, ShaderType::COMPUTESHADER, 0);
	LowLvlGfx::Bind(m_eq2cubeCS);

	LowLvlGfx::Context()->Dispatch(cubeSideLength / 32, cubeSideLength / 32, 6);
	LowLvlGfx::BindUAVs(); // unbind
	//LowLvlGfx::Context()->GenerateMips(cubeMap->srv.Get());

	return cubeMap;
}
