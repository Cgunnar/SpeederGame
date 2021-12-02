#pragma once
#include "DX11.h"
#include "GraphicsResources.h"
#include <memory>


struct BufferDesc
{
	uint32_t size;
	enum class USAGE
	{
		DYNAMIC = 1,
		DEFAULT = 2,

	} usage;

};




class LowLvlGfx
{

	// I think the syntax for calling static functions look nice, deal with it.

public:
	LowLvlGfx() = delete;
	LowLvlGfx(const LowLvlGfx& other) = delete;
	LowLvlGfx& operator=(const LowLvlGfx& other) = delete;
	static void ResizeWindow(Resolution newRes);

	static void Init(HWND hwnd, Resolution res);
	static void Destroy();
	static void OnResize(Resolution res);
	static bool IsValid();
	static void EnterFullScreen();
	static void LeaveFullScreen();
	static bool IsFullScreen();
	static Microsoft::WRL::ComPtr<IDXGISwapChain>& SwapChain();
	static Microsoft::WRL::ComPtr<ID3D11Device>& Device();
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Context();
	static void SetViewPort(Resolution res);

	//create
	static std::shared_ptr<Texture2D> CreateTexture2D(D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr, bool fixedRes = true);
	static void CreateSRV(std::shared_ptr<Texture2D> tex2d, D3D11_SHADER_RESOURCE_VIEW_DESC* desc = nullptr);
	static Shader CreateShader(const std::string& path, ShaderType type);
	static VertexBuffer CreateVertexBuffer(const float* data, uint32_t byteWidth, uint32_t stride, uint32_t offset = 0);
	static IndexBuffer CreateIndexBuffer(const uint32_t* data, uint32_t indexCount, uint32_t offset = 0);
	static ConstantBuffer CreateConstantBuffer(BufferDesc desc, void* data = nullptr);
	static Sampler CreateSampler(D3D11_SAMPLER_DESC desc);

	//bind
	static void Bind(const Shader& shader);
	static void Bind(const Sampler& sampler, ShaderType shaderType, uint32_t bindSlot);
	static void Bind(const VertexBuffer& vertexBuffer);
	static void Bind(const IndexBuffer& indexBuffer);
	static void Bind(ConstantBuffer cBuff, ShaderType shaderType, uint32_t bindSlot);
	static void BindRTVs(std::vector<std::shared_ptr<Texture2D>> rtvs = {}, std::shared_ptr<Texture2D> dsv = nullptr);
	static void BindRTVsAndUAVs(std::vector<std::shared_ptr<Texture2D>> rtvs,
		std::vector<std::shared_ptr<Texture2D>> uavs, std::shared_ptr<Texture2D> dsv = nullptr);
	static void BindUAV(std::shared_ptr<Texture2D> uav, ShaderType shaderType, uint32_t bindSlot);
	static void BindSRV(std::shared_ptr<Texture2D> srv, ShaderType shaderType, uint32_t bindSlot);

	static void UpdateBuffer(ConstantBuffer cBuff, void* data);
	static void ClearRTV(float rgba[4], std::shared_ptr<Texture2D> rtv);
	static void ClearRTV(float r, float g, float b, float a, std::shared_ptr<Texture2D> rtv);
	static void ClearDSV(std::shared_ptr<Texture2D> dsv);

	//draws
	static void Draw(uint32_t vertexCount);
	static void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);

	static void Present(bool vsync = false);

	static std::shared_ptr<Texture2D> GetBackBuffer();
	static std::shared_ptr<Texture2D> GetDepthBuffer();

private:
	static DX11* s_dx11;
};

