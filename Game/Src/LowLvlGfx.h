#pragma once
#include "DX11.h"
#include "Shader.h"
#include "GraphicsResources.h"
#include <memory>

enum class Topology
{
	TRIANGLELIST = 0,
};




struct BufferDesc
{
	uint32_t size;
	enum class USAGE
	{
		DYNAMIC = 1,
		DEFAULT = 2,

	} usage;

};

class ConstantBuffer
{
	friend LowLvlGfx;
private:
	ConstantBuffer(uint32_t size) : m_size(size) {}
	uint32_t m_size;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

public:
};


class LowLvlGfx
{
public:
	LowLvlGfx() = delete;
	LowLvlGfx(const LowLvlGfx& other) = delete;
	LowLvlGfx& operator=(const LowLvlGfx& other) = delete;
	static void ResizeWindow(Resolution newRes);

	static void Init(HWND hwnd, Resolution res);
	static void Destroy();
	static void SetViewPort(Resolution res);
	static std::shared_ptr<Texture2D> CreateTexture2D(D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr, bool fixedRes = true);
	static void SetTopology(Topology t);
	static Shader CreateShader(const std::string& path, ShaderType type);
	static VertexBuffer CreateVertexBuffer(const float* data, uint32_t size, uint32_t stride, uint32_t offset = 0);
	static void Bind(Shader shader);
	static void Bind(VertexBuffer vertexBuffer);
	static ConstantBuffer CreateConstantBuffer(BufferDesc desc, void* data = nullptr);
	static void Bind(ConstantBuffer cBuff, ShaderType shaderType, uint32_t bindSlot);
	static void UpdateBuffer(ConstantBuffer cBuff, void* data);
	static void Draw(uint32_t vertexCount);
	static void ClearRTV(float rgba[4], std::shared_ptr<Texture2D> rtv);
	static void ClearRTV(float r, float g, float b, float a, std::shared_ptr<Texture2D> rtv);
	static void ClearDSV(std::shared_ptr<Texture2D> dsv);
	static void Present(bool vsync = false);

	static std::shared_ptr<Texture2D> GetBackBuffer();
	static std::shared_ptr<Texture2D> GetDepthBuffer();

private:
	static DX11* s_dx11;
};

