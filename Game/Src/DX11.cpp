#include "DX11.h"
#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>

#include <assert.h>
#include <sstream>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment(lib, "dxguid.lib")
using namespace Microsoft::WRL;





DX11::DX11(HWND hwnd, Resolution res)
{
	CreateDeviceAndSwapChain(hwnd, res);

	m_backBuffer = std::make_shared<Texture2D>();
	m_zBuffer = std::make_shared<Texture2D>();
	
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &m_backBuffer->buffer);
	assert(SUCCEEDED(hr));

	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.Texture2D.MipSlice = 0;
	hr = m_device->CreateRenderTargetView(m_backBuffer->buffer.Get(), &desc, &m_backBuffer->rtv);
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
DX11::~DX11()
{

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



void  DX11::CreateDeviceAndSwapChain(HWND hwnd, Resolution res)
{
	UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, nullptr, 0, D3D11_SDK_VERSION, &m_device, nullptr, &m_context);
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


	IDXGIDevice* pDXGIDevice = nullptr;
	hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
	assert(SUCCEEDED(hr));

	IDXGIAdapter* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	assert(SUCCEEDED(hr));

	IDXGIFactory* pIDXGIFactory = nullptr;
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
	assert(SUCCEEDED(hr));

	hr = pIDXGIFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, &m_swapChain);
	assert(SUCCEEDED(hr));

	pIDXGIFactory->Release();
	pDXGIAdapter->Release();
	pDXGIDevice->Release();
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