#include "pch.hpp"
#include "SkyBox.h"
#include "ReadImg.hpp"
#include "LowLvlGfx.h"
#include <stb_image.h>


void SkyBox::Init(const std::string& path)
{
	if (path.substr(path.length() - 4, 4) == ".hdr")
		InitCubeMapHDR(path);
	else
		InitCubeMapLDR(path);

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
	if (m_ldr)
	{
		LowLvlGfx::UpdateBuffer(rendRes.m_worldMatrixCB, &m_rotation);
		LowLvlGfx::Bind(rendRes.m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
		LowLvlGfx::Bind(rendRes.m_vpCB, ShaderType::VERTEXSHADER, 1);
		LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }); // this should unbind the z-buffer
		LowLvlGfx::Bind(m_skyBoxVS);
		LowLvlGfx::Bind(m_skyBoxPS);
		LowLvlGfx::BindSRV(m_skyBoxCubeMap, ShaderType::PIXELSHADER, 4);
		LowLvlGfx::Bind(m_sampler, ShaderType::PIXELSHADER, 1);
		LowLvlGfx::Bind(m_rasterizer);
	}
}

void SkyBox::SetRotation(rfm::Matrix rot)
{
	m_rotation = rot;
}

void SkyBox::InitCubeMapLDR(const std::string& path)
{
	assert(!m_ldr && !m_hdr);
	m_ldr = true;

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

	m_skyBoxVS = LowLvlGfx::CreateShader("Src/Shaders/VS_SkyBox.hlsl", ShaderType::VERTEXSHADER);
	m_skyBoxPS = LowLvlGfx::CreateShader("Src/Shaders/PS_SkyBox.hlsl", ShaderType::PIXELSHADER);


	D3D11_RASTERIZER_DESC rzDesc = {};
	rzDesc.CullMode = D3D11_CULL_BACK;
	rzDesc.FillMode = D3D11_FILL_SOLID;
	rzDesc.FrontCounterClockwise = true;
	m_rasterizer = LowLvlGfx::Create(rzDesc);
}

void SkyBox::InitCubeMapHDR(const std::string& path)
{
	assert(std::filesystem::exists(path));
	assert(!m_ldr && !m_hdr);
	m_hdr = true;

	int w, h, c;
	float* hdrImage = stbi_loadf(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
	assert(hdrImage);




	stbi_image_free(hdrImage);
}
