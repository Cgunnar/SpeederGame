#include "pch.hpp"
#include "DX11.h"

#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <backends\imgui_impl_win32.h>
#include <backends\imgui_impl_dx11.h>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxguid.lib")
#pragma comment( lib, "dxgi.lib")


extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


using namespace Microsoft::WRL;




DX11::DX11(HWND hwnd, Resolution res) : m_resolution(res)
{
	CreateDeviceAndSwapChain(hwnd, res);
	m_backBuffer = std::make_shared<Texture2D>();
	m_zBuffer = std::make_shared<Texture2D>();
	CreateRTVandDSV();

	CheckMonitorRes();
	//imgui
	ImGui_ImplDX11_Init(m_device.Get(), m_context.Get());

}
DX11::~DX11()
{
	ImGui_ImplDX11_Shutdown();

	m_context->ClearState();
	m_context->Flush();
}

void DX11::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}


void DX11::EndFrame(bool vsync)
{

	m_context->OMSetRenderTargets(1, m_backBufferViewNoSRGB.GetAddressOf(), nullptr);
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	m_swapChain->Present(vsync ? 1 : 0, 0);
}

void DX11::SetViewPort(Resolution res)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)res.width;
	vp.Height = (float)res.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &vp);
}

void DX11::OnResize(Resolution res)
{

	m_context->OMSetRenderTargets(0, nullptr, nullptr);

	m_context->ClearState();
	m_context->Flush();

	if (m_backBufferViewNoSRGB.Reset()) assert(false);
	if (m_backBuffer->rtv.Reset()) assert(false);
	if (m_backBuffer->buffer.Reset()) assert(false);

	if (m_zBuffer->dsv.Reset()) assert(false);
	if (m_zBuffer->buffer.Reset()) assert(false);

	HRESULT hr;
#ifdef DEBUG
	std::cout << "ResizeBuffers width: " << res.width << " height: " << res.height << std::endl;
#endif // DEBUG

	hr = m_swapChain->ResizeBuffers(0, res.width, res.height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
#ifdef D3D11_DEBUG
	if (!SUCCEEDED(hr))
	{
		m_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
#endif // D3D11_DEBUG
	assert(SUCCEEDED(hr));

	CreateRTVandDSV();
	SetViewPort(res);
	m_resolution = res;
}

void DX11::ResizeTarget(Resolution res)
{
	DXGI_MODE_DESC modeDesc = {};

	DXGI_MODE_DESC prefModeDesc = {};

	prefModeDesc.Width = res.width;
	prefModeDesc.Height = res.height;
	prefModeDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	prefModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;

	IDXGIOutput* outPut = nullptr;
	HRESULT hr = m_swapChain->GetContainingOutput(&outPut);
	assert(SUCCEEDED(hr));

	hr = outPut->FindClosestMatchingMode(&prefModeDesc, &modeDesc, m_device.Get());
	assert(SUCCEEDED(hr));

#ifdef DEBUG
	std::string debugOut = "FindClosestMatchingMode:\tResolution: " + std::to_string(modeDesc.Width) + "x" + std::to_string(modeDesc.Height) +
		"\tRefreshRate: " + std::to_string((float)modeDesc.RefreshRate.Numerator / (float)modeDesc.RefreshRate.Denominator) + "\n";
	std::cout << debugOut;
#endif // DEBUG

	hr = m_swapChain->ResizeTarget(&modeDesc);
	assert(SUCCEEDED(hr));
	outPut->Release();
}

void DX11::CheckMonitorRes()
{

	m_nativeRes = {};
	IDXGIOutput* outPut;
	HRESULT hr = m_swapChain->GetContainingOutput(&outPut);
	assert(SUCCEEDED(hr));
	DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;

	UINT numModes = 0;

	hr = outPut->GetDisplayModeList(format, 0, &numModes, 0);
	assert(SUCCEEDED(hr));
	std::vector<DXGI_MODE_DESC> modeVec;
	modeVec.resize(numModes);
	hr = outPut->GetDisplayModeList(format, 0, &numModes, modeVec.data());
	assert(SUCCEEDED(hr));

	std::sort(modeVec.begin(), modeVec.end(), [](auto a, auto b) {
		if (a.Width * a.Height > b.Width * b.Height) return true;
		if (a.Width * a.Height < b.Width * b.Height) return false;
		return (float)(a.RefreshRate.Numerator / (float)a.RefreshRate.Denominator) >
			(float)(b.RefreshRate.Numerator / (float)b.RefreshRate.Denominator);
		});

	m_nativeRes.width = modeVec.front().Width;
	m_nativeRes.height = modeVec.front().Height;
	float hz = (float)modeVec.front().RefreshRate.Numerator / (float)modeVec.front().RefreshRate.Denominator;

#ifdef DEBUG
	DXGI_OUTPUT_DESC outDesc;
	outPut->GetDesc(&outDesc);
	std::wstring name = outDesc.DeviceName;
	std::wstring debugOut = name + L" Best Mode: resolution: " + std::to_wstring(m_nativeRes.width) + L"x" + std::to_wstring(m_nativeRes.height) +
		L" hz: " + std::to_wstring(hz) + L"\n";
	std::wcout << debugOut;
#endif // DEBUG

	outPut->Release();
}

uint32_t DX11::CreateShader(Microsoft::WRL::ComPtr<ID3DBlob> blob, ShaderType type)
{
	ShaderDX shader;
	shader.blob = blob;
	shader.type = type;
	HRESULT hr{};
	switch (type)
	{
	case ShaderType::VERTEXSHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11VertexShader>();
		hr = m_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11VertexShader>>(shader.shaderVariant));
		break;
	case ShaderType::HULLSHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11HullShader>();
		hr = m_device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11HullShader>>(shader.shaderVariant));
		break;
	case ShaderType::DOMAINSHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11DomainShader>();
		hr = m_device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11DomainShader>>(shader.shaderVariant));
		break;
	case ShaderType::GEOMETRYSHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11GeometryShader>();
		hr = m_device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11GeometryShader>>(shader.shaderVariant));
		break;
	case ShaderType::PIXELSHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11PixelShader>();
		hr = m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11PixelShader>>(shader.shaderVariant));
		break;
	case ShaderType::COMPUTESHADER:
		shader.shaderVariant = Microsoft::WRL::ComPtr<ID3D11ComputeShader>();
		hr = m_device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
			&std::get<Microsoft::WRL::ComPtr<ID3D11ComputeShader>>(shader.shaderVariant));
		break;
	}
	assert(SUCCEEDED(hr));

	hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_ID3D11ShaderReflection, &shader.reflection);
	assert(SUCCEEDED(hr));

	uint32_t id = (uint32_t)m_shaders.size();
	m_shaders.push_back(shader);

	if (shader.type == ShaderType::VERTEXSHADER)
	{
		CreateInputLayout(id);
	}
	return id;
}

void DX11::CreateInputLayout(uint32_t shaderID)
{
	auto& VSReflection = m_shaders[shaderID].reflection;

	D3D11_SHADER_DESC desc;
	HRESULT hr = VSReflection->GetDesc(&desc);
	assert(SUCCEEDED(hr));
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescriptors;

	for (uint32_t i = 0; i < desc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		HRESULT hr = VSReflection->GetInputParameterDesc(i, &paramDesc);
		assert(SUCCEEDED(hr));

		D3D11_INPUT_ELEMENT_DESC elemDesc;
		elemDesc.SemanticName = paramDesc.SemanticName;
		elemDesc.SemanticIndex = paramDesc.SemanticIndex;
		elemDesc.InputSlot = 0;
		elemDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elemDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elemDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elemDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elemDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elemDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elemDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elemDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elemDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elemDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elemDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elemDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elemDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		elementDescriptors.push_back(elemDesc);
	}

	auto& blob = m_shaders[shaderID].blob;
	m_inputLayers[shaderID] = Microsoft::WRL::ComPtr<ID3D11InputLayout>();
	hr = m_device->CreateInputLayout(elementDescriptors.data(), (uint32_t)elementDescriptors.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayers[shaderID]);
	assert(SUCCEEDED(hr));
}

bool DX11::IsFullScreen()
{
	HRESULT hr = this->m_swapChain.Get()->GetFullscreenState(&this->m_fullScreen, nullptr);
	assert(SUCCEEDED(hr));
	return (bool)this->m_fullScreen;
}



void  DX11::CreateDeviceAndSwapChain(HWND hwnd, Resolution res)
{

	IDXGIFactory6* pIDXGIFactory6 = nullptr;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory6), (void**)&pIDXGIFactory6);
	IDXGIAdapter4* pDXGIAdapter4;
	
	hr = pIDXGIFactory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		__uuidof(IDXGIAdapter4), (void**)&pDXGIAdapter4);
	DXGI_ADAPTER_DESC3 Adesc;
	pDXGIAdapter4->GetDesc3(&Adesc);
	DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
	pDXGIAdapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP::DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
	std::wcout << Adesc.Description << std::endl;

	UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, nullptr, 0, D3D11_SDK_VERSION, &m_device, nullptr, &m_context);
	assert(SUCCEEDED(hr));

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = res.width;
	swapChainDesc.BufferDesc.Height = res.height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

#ifdef D3D11_DEBUG
	hr = this->m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_debug);
	assert(SUCCEEDED(hr));
#endif // D3D11_DEBUG

	hr = pIDXGIFactory6->CreateSwapChain(m_device.Get(), &swapChainDesc, &m_swapChain);
	assert(SUCCEEDED(hr));

	pIDXGIFactory6->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	pDXGIAdapter4->Release();
	pIDXGIFactory6->Release();
}

void DX11::CreateRTVandDSV()
{
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &m_backBuffer->buffer);
	assert(SUCCEEDED(hr));

	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.Texture2D.MipSlice = 0;
	hr = m_device->CreateRenderTargetView(m_backBuffer->buffer.Get(), &desc, &m_backBuffer->rtv);
	assert(SUCCEEDED(hr));

	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	hr = m_device->CreateRenderTargetView(m_backBuffer->buffer.Get(), &desc, &m_backBufferViewNoSRGB);
	assert(SUCCEEDED(hr));

	D3D11_TEXTURE2D_DESC zbufferDesc;
	m_backBuffer->buffer->GetDesc(&zbufferDesc);
	//bufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	zbufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	zbufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	zbufferDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = m_device->CreateTexture2D(&zbufferDesc, nullptr, &m_zBuffer->buffer);
	assert(SUCCEEDED(hr));


	hr = m_device->CreateDepthStencilView(m_zBuffer->buffer.Get(), nullptr, &m_zBuffer->dsv);
	assert(SUCCEEDED(hr));
}



Microsoft::WRL::ComPtr<ID3DBlob> DX11::CompileShader(const std::string& path, const std::string& shaderModel, const std::string& entryPoint)
{
	std::wstringstream wPathStream;
	wPathStream << path.c_str();
	std::wstring wPath = wPathStream.str();

	UINT sFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	sFlags |= D3DCOMPILE_DEBUG;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT	hr = D3DCompileFromFile
	(
		wPath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(),
		shaderModel.c_str(),
		sFlags,
		0,
		&shaderBlob,
		&errorBlob
	);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		if (shaderBlob)
		{
			shaderBlob.Reset();
		}
		assert(false);
	}
	return shaderBlob;
}