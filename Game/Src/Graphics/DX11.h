#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>

#include "utilityTypes.h"
#include "GraphicsResources.h"

//#define D3D11_DEBUG

class LowLvlGfx;

enum class ShaderType
{
	NONE = 0,
	VERTEXSHADER = 1,
	HULLSHADER = 2,
	DOMAINSHADER = 4,
	GEOMETRYSHADER = 8,
	PIXELSHADER = 16,
	COMPUTESHADER = 32
};
inline ShaderType operator &(ShaderType l, ShaderType r)
{
	return (ShaderType)((int)l & (int)r);
}
inline ShaderType operator |(ShaderType l, ShaderType r)
{
	return (ShaderType)((int)l | (int)r);
}

class DX11
{
	friend LowLvlGfx;

	struct ShaderDX
	{
		ShaderType type = ShaderType::NONE;
		Microsoft::WRL::ComPtr<ID3DBlob> blob = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
		std::variant<
			Microsoft::WRL::ComPtr<ID3D11VertexShader>,
			Microsoft::WRL::ComPtr<ID3D11HullShader>,
			Microsoft::WRL::ComPtr<ID3D11DomainShader>,
			Microsoft::WRL::ComPtr<ID3D11GeometryShader>,
			Microsoft::WRL::ComPtr<ID3D11PixelShader>,
			Microsoft::WRL::ComPtr<ID3D11ComputeShader>> shaderVariant;
	};

	DX11() = delete;
	DX11(HWND hwnd, Resolution res);
	~DX11();
	void BeginFrame();
	void EndFrame(bool vsync);
	void SetViewPort(Resolution res);
	DX11(const DX11& other) = delete;
	DX11& operator=(const DX11& other) = delete;

	void CreateDeviceAndSwapChain(HWND hwnd, Resolution res);
	void CreateRTVandDSV();
	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::string& path, const std::string& shaderModel, const std::string& entryPoint);
	
#ifdef D3D11_DEBUG
	ID3D11Debug* m_debug = nullptr;
#endif // D3D11_DEBUG


	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
	
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferViewNoSRGB;
	std::shared_ptr<Texture2D> m_backBuffer;
	std::shared_ptr<Texture2D> m_zBuffer;

	BOOL m_fullScreen = false;
	Resolution m_nativeRes;
	Resolution m_resolution;

	
	std::vector<ShaderDX> m_shaders;
	std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3D11InputLayout>> m_inputLayers;

	std::vector<Texture2D> m_allTexture2Ds;

	void OnResize(Resolution res);
	void CheckMonitorRes();
	uint32_t CreateShader(Microsoft::WRL::ComPtr<ID3DBlob> blob, ShaderType type);
	void CreateInputLayout(uint32_t shaderID);

	void ResizeTarget(Resolution res);
	bool IsFullScreen();
	Microsoft::WRL::ComPtr<IDXGIOutput> GetOutPut();

};
