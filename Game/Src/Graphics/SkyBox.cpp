#include "pch.hpp"
#include "SkyBox.h"
#include "ReadImg.hpp"
#include "LowLvlGfx.h"


void SkyBox::Init(const std::string& path)
{
	//assert(path.substr(path.length() - 4, 4) == ".hdr");


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

void SkyBox::Bind(SharedRenderResources& rendRes)
{
	LowLvlGfx::UpdateBuffer(rendRes.m_worldMatrixCB, &m_rotation);
	LowLvlGfx::Bind(rendRes.m_worldMatrixCB, ShaderType::VERTEXSHADER, 0);
	LowLvlGfx::Bind(rendRes.m_vpCB, ShaderType::VERTEXSHADER, 1);
	LowLvlGfx::BindRTVs({ LowLvlGfx::GetBackBuffer() }); // this should unbind the z-buffer
	LowLvlGfx::Bind(m_skyBoxVS);
	LowLvlGfx::Bind(m_skyBoxPS);
	LowLvlGfx::BindSRV(m_skyBoxCubeMap, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(rendRes.m_linearWrapSampler, ShaderType::PIXELSHADER, 0);
	LowLvlGfx::Bind(m_rasterizer);
}
