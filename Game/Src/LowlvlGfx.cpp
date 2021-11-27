#include "LowLvlGfx.h"
#include <assert.h>

using namespace Microsoft::WRL;

DX11* LowLvlGfx::s_dx11 = nullptr;

void LowLvlGfx::ResizeWindow(Resolution newRes)
{
}

void LowLvlGfx::Init(HWND hwnd, Resolution res)
{
	assert(!s_dx11);
	s_dx11 = new DX11(hwnd, res);
}

void LowLvlGfx::Destroy()
{
	assert(s_dx11);
	delete s_dx11;
	s_dx11 = nullptr;
}

void LowLvlGfx::SetViewPort(Resolution res)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)res.width;
	vp.Height = (float)res.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	s_dx11->m_context->RSSetViewports(1, &vp);
}

std::shared_ptr<Texture2D> LowLvlGfx::CreateTexture2D(D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data, bool fixedRes)
{
	std::shared_ptr<Texture2D> tex2d = std::make_shared<Texture2D>();
	tex2d->fixedRes = fixedRes;

	HRESULT hr = s_dx11->m_device->CreateTexture2D(&desc, data, &tex2d->buffer);
	assert(SUCCEEDED(hr));

	return tex2d;
}

void LowLvlGfx::SetTopology(Topology t)
{
	switch (t)
	{
	case Topology::TRIANGLELIST:
		s_dx11->m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	default:
		break;
	}
}

Shader LowLvlGfx::CreateShader(const std::string& path, ShaderType type)
{
	std::string shaderModel;
	switch (type)
	{
	case ShaderType::VERTEXSHADER: shaderModel = "vs_5_0"; break;
	case ShaderType::HULLSHADER: shaderModel = "hs_5_0"; break;
	case ShaderType::DOMAINSHADER: shaderModel = "ds_5_0"; break;
	case ShaderType::GEOMETRYSHADER: shaderModel = "gs_5_0"; break;
	case ShaderType::PIXELSHADER: shaderModel = "ps_5_0"; break;
	case ShaderType::COMPUTESHADER: shaderModel = "cs_5_0"; break;
	}
	auto blob = s_dx11->CompileShader(path, shaderModel, "main");
	uint32_t id = s_dx11->CreateShader(blob, type);
	return Shader(id);
}

VertexBuffer LowLvlGfx::CreateVertexBuffer(const float* data, uint32_t size, uint32_t stride, uint32_t offset)
{
	VertexBuffer vs;
	vs.vertexStride = stride;
	vs.vertexOffset = offset;
	vs.vertexCount = size / stride;
	vs.vertexBuffer = ComPtr<ID3D11Buffer>();

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = size;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA srData = { 0 };

	srData.pSysMem = data;

	HRESULT hr = s_dx11->m_device->CreateBuffer(&desc, &srData, &vs.vertexBuffer);
	assert(SUCCEEDED(hr));

	return vs;
}
void LowLvlGfx::Bind(Shader shader)
{
	auto& s = s_dx11->m_shaders[shader.m_id];
	switch (s.type)
	{
	case ShaderType::VERTEXSHADER: 
		s_dx11->m_context->VSSetShader(std::get<ComPtr<ID3D11VertexShader>>(s.shaderVariant).Get(), nullptr, 0);
		s_dx11->m_context->IASetInputLayout(s_dx11->m_inputLayers[shader.m_id].Get());
		break;
	case ShaderType::HULLSHADER: s_dx11->m_context->HSSetShader(std::get<ComPtr<ID3D11HullShader>>(s.shaderVariant).Get(), nullptr, 0); break;
	case ShaderType::DOMAINSHADER: s_dx11->m_context->DSSetShader(std::get<ComPtr<ID3D11DomainShader>>(s.shaderVariant).Get(), nullptr, 0); break;
	case ShaderType::GEOMETRYSHADER: s_dx11->m_context->GSSetShader(std::get<ComPtr<ID3D11GeometryShader>>(s.shaderVariant).Get(), nullptr, 0); break;
	case ShaderType::PIXELSHADER: s_dx11->m_context->PSSetShader(std::get<ComPtr<ID3D11PixelShader>>(s.shaderVariant).Get(), nullptr, 0); break;
	case ShaderType::COMPUTESHADER: s_dx11->m_context->CSSetShader(std::get<ComPtr<ID3D11ComputeShader>>(s.shaderVariant).Get(), nullptr, 0); break;
	}
}

void LowLvlGfx::Bind(VertexBuffer vertexBuffer)
{
	s_dx11->m_context->IASetVertexBuffers(0, 1, vertexBuffer.vertexBuffer.GetAddressOf(), &vertexBuffer.vertexStride, &vertexBuffer.vertexOffset);
}

ConstantBuffer LowLvlGfx::CreateConstantBuffer(BufferDesc desc, void* data)
{
	ConstantBuffer buffer = ConstantBuffer(desc.size);
	D3D11_BUFFER_DESC cdesc;
	cdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cdesc.ByteWidth = desc.size;
	cdesc.MiscFlags = 0;
	cdesc.StructureByteStride = 0;

	switch (desc.usage)
	{
	case BufferDesc::USAGE::DEFAULT:
		cdesc.Usage = D3D11_USAGE_DEFAULT;
		cdesc.CPUAccessFlags = 0;
		break;
	case BufferDesc::USAGE::DYNAMIC:
		cdesc.Usage = D3D11_USAGE_DYNAMIC;
		cdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}
	
	HRESULT hr;
	if (data)
	{
		D3D11_SUBRESOURCE_DATA sdata;
		sdata.pSysMem = data;
		hr = s_dx11->m_device->CreateBuffer(&cdesc, &sdata, &buffer.m_buffer);
	}
	else
	{
		hr = s_dx11->m_device->CreateBuffer(&cdesc, nullptr, &buffer.m_buffer);
	}
	assert(SUCCEEDED(hr));

	return buffer;
}

void LowLvlGfx::Bind(ConstantBuffer cBuff, ShaderType shaderType, uint32_t bindSlot)
{
	switch (shaderType)
	{
	case ShaderType::VERTEXSHADER: s_dx11->m_context->VSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	case ShaderType::HULLSHADER: s_dx11->m_context->HSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	case ShaderType::DOMAINSHADER: s_dx11->m_context->DSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	case ShaderType::GEOMETRYSHADER: s_dx11->m_context->GSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	case ShaderType::PIXELSHADER: s_dx11->m_context->PSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	case ShaderType::COMPUTESHADER: s_dx11->m_context->CSSetConstantBuffers(bindSlot, 1, cBuff.m_buffer.GetAddressOf()); break;
	}
}

void LowLvlGfx::UpdateBuffer(ConstantBuffer cbuff, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubRes;
	s_dx11->m_context->Map(cbuff.m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);
	memcpy(mappedSubRes.pData, data, cbuff.m_size);
	s_dx11->m_context->Unmap(cbuff.m_buffer.Get(), 0);
}

void LowLvlGfx::Draw(uint32_t vertexCount)
{
	s_dx11->m_context->OMSetRenderTargets(1, GetBackBuffer()->rtv.GetAddressOf(), GetDepthBuffer()->dsv.Get());
	s_dx11->m_context->Draw(vertexCount, 0);
}

void LowLvlGfx::ClearRTV(float rgba[4], std::shared_ptr<Texture2D> rtv)
{
	s_dx11->m_context->ClearRenderTargetView(rtv->rtv.Get(), rgba);
}void LowLvlGfx::ClearRTV(float r, float g, float b, float a, std::shared_ptr<Texture2D> rtv)
{
	float color[]{ r, g, b, a };
	s_dx11->m_context->ClearRenderTargetView(rtv->rtv.Get(), color);
}

void LowLvlGfx::ClearDSV(std::shared_ptr<Texture2D> dsv)
{
	s_dx11->m_context->ClearDepthStencilView(dsv->dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void LowLvlGfx::Present(bool vsync)
{
	s_dx11->m_swapChain->Present(vsync ? 1 : 0, 0);
}

std::shared_ptr<Texture2D> LowLvlGfx::GetBackBuffer()
{
	return s_dx11->m_backBuffer;
}

std::shared_ptr<Texture2D> LowLvlGfx::GetDepthBuffer()
{
	return s_dx11->m_zBuffer;
}
