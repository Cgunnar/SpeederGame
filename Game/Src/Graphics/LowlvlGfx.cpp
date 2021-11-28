#include "LowLvlGfx.h"
#include <assert.h>
#include <iostream>

using namespace Microsoft::WRL;

DX11* LowLvlGfx::s_dx11 = nullptr;



void LowLvlGfx::Init(HWND hwnd, Resolution res)
{
	assert(!s_dx11);
	s_dx11 = new DX11(hwnd, res);
}

void LowLvlGfx::Destroy()
{
	assert(s_dx11);
#ifdef D3D11_DEBUG
	ID3D11Debug* debug = s_dx11->m_debug;
#endif // D3D11_DEBUG
	delete s_dx11;
	s_dx11 = nullptr;


#ifdef D3D11_DEBUG
	//debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	debug->Release();
#endif // DD3D11_DEBUGEBUG
}

bool LowLvlGfx::IsValid()
{
	return s_dx11 != nullptr;
}

void LowLvlGfx::OnResize(Resolution res)
{
	if (!s_dx11) return;
	s_dx11->OnResize(res);
}



void LowLvlGfx::ResizeWindow(Resolution newRes)
{
	if (IsFullScreen()) LeaveFullScreen();
	s_dx11->ResizeTarget(newRes);
}

void LowLvlGfx::EnterFullScreen()
{
	if (IsFullScreen()) return;
#ifdef DEBUG
	std::cout << "EnterFullScreen\n";
#endif // DEBUG

	s_dx11->ResizeTarget(s_dx11->m_nativeRes);
	s_dx11->m_swapChain->SetFullscreenState(true, nullptr);
}

void LowLvlGfx::LeaveFullScreen()
{
	if (!IsFullScreen()) return;
#ifdef DEBUG
	std::cout << "LeaveFullScreen\n";
#endif // DEBUG
	s_dx11->m_swapChain->SetFullscreenState(false, nullptr);
	s_dx11->ResizeTarget({ 1280, 720 });
}

bool LowLvlGfx::IsFullScreen()
{
	return s_dx11->IsFullScreen();
}

Microsoft::WRL::ComPtr<IDXGISwapChain>& LowLvlGfx::SwapChain()
{
	return s_dx11->m_swapChain;
}

Microsoft::WRL::ComPtr<ID3D11Device>& LowLvlGfx::Device()
{
	return s_dx11->m_device;
}

Microsoft::WRL::ComPtr<ID3D11DeviceContext>& LowLvlGfx::Context()
{
	return s_dx11->m_context;
}

void LowLvlGfx::SetViewPort(Resolution res)
{
	s_dx11->SetViewPort(res);
}

std::shared_ptr<Texture2D> LowLvlGfx::CreateTexture2D(D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data, bool fixedRes)
{
	std::shared_ptr<Texture2D> tex2d = std::make_shared<Texture2D>();
	tex2d->fixedRes = fixedRes;

	HRESULT hr = s_dx11->m_device->CreateTexture2D(&desc, data, &tex2d->buffer);
	assert(SUCCEEDED(hr));

	return tex2d;
}

void LowLvlGfx::CreateSRV(std::shared_ptr<Texture2D> tex2d, D3D11_SHADER_RESOURCE_VIEW_DESC* desc)
{
	HRESULT hr = s_dx11->m_device->CreateShaderResourceView(tex2d->buffer.Get(), desc, &tex2d->srv);
	assert(SUCCEEDED(hr));

	D3D11_TEXTURE2D_DESC desc2d;
	tex2d->buffer->GetDesc(&desc2d);
	if (desc2d.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
	{
		s_dx11->m_context->GenerateMips(tex2d->srv.Get());
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

VertexBuffer LowLvlGfx::CreateVertexBuffer(const float* data, uint32_t byteWidth, uint32_t stride, uint32_t offset)
{
	VertexBuffer vs;
	vs.vertexStride = stride;
	vs.vertexOffset = offset;
	vs.vertexCount = byteWidth / stride;
	vs.vertexBuffer = ComPtr<ID3D11Buffer>();

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = byteWidth;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA srData = { 0 };

	srData.pSysMem = data;

	HRESULT hr = s_dx11->m_device->CreateBuffer(&desc, &srData, &vs.vertexBuffer);
	assert(SUCCEEDED(hr));

	return vs;
}
IndexBuffer LowLvlGfx::CreateIndexBuffer(const uint32_t* data, uint32_t indexCount, uint32_t offset)
{
	D3D11_BUFFER_DESC indexDesc;
	indexDesc.ByteWidth = indexCount * sizeof(uint32_t);
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subres;
	subres.pSysMem = data;

	IndexBuffer ib;
	ib.startIndexLocation = 0;
	ib.indexCount = indexCount;
	HRESULT hr = s_dx11->m_device->CreateBuffer(&indexDesc, &subres, &ib.m_indexBuffer);
	assert(SUCCEEDED(hr));

	return ib;
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

void LowLvlGfx::Bind(const VertexBuffer& vertexBuffer)
{
	s_dx11->m_context->IASetVertexBuffers(0, 1, vertexBuffer.vertexBuffer.GetAddressOf(), &vertexBuffer.vertexStride, &vertexBuffer.vertexOffset);
}

void LowLvlGfx::Bind(const IndexBuffer& indexBuffer)
{
	s_dx11->m_context->IASetIndexBuffer(indexBuffer.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
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

void LowLvlGfx::BindRTVs(std::vector<std::shared_ptr<Texture2D>> rtvs, std::shared_ptr<Texture2D> dsv)
{
	if (rtvs.empty() && !dsv)
	{
		s_dx11->m_context->OMSetRenderTargets(0, nullptr, nullptr);
		return;
	}

	assert(rtvs.size() <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	auto dsvPtr = dsv ? dsv->dsv.Get() : nullptr;
	ID3D11RenderTargetView* views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	for (int i = 0; i < rtvs.size(); i++)
	{
		views[i] = rtvs[i]->rtv.Get();
	}
	s_dx11->m_context->OMSetRenderTargets((UINT)rtvs.size(), views, dsvPtr);
}

void LowLvlGfx::BindRTVsAndUAVs(std::vector<std::shared_ptr<Texture2D>> rtvs, std::vector<std::shared_ptr<Texture2D>> uavs, std::shared_ptr<Texture2D> dsv)
{
	assert(rtvs.size() + uavs.size() <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	assert(false);
}

void LowLvlGfx::BindUAV(std::shared_ptr<Texture2D> uav, ShaderType shaderType, uint32_t bindSlot)
{													
	assert(false);
}

void LowLvlGfx::BindSRV(std::shared_ptr<Texture2D> srv, ShaderType shaderType, uint32_t bindSlot)
{
	assert(false);
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
	s_dx11->m_context->Draw(vertexCount, 0);
}

void LowLvlGfx::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	s_dx11->m_context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
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
