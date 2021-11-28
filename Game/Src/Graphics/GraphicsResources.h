#pragma once

#include <wrl.h>
#include <d3d11.h>
#define NOMINMAX
#include <windows.h>
#include <cstdint>

class LowLvlGfx;
class DX11;


namespace standardSamplers
{
	extern const D3D11_SAMPLER_DESC g_linear_wrap;
}

class Shader
{
	friend LowLvlGfx;
public:

private:
	Shader(uint32_t id) : m_id(id) {}
	uint32_t m_id;
};

class VertexBuffer
{
public:
	VertexBuffer() = default;
	uint32_t GetVertexCount() const { return vertexCount; }
private:
	friend LowLvlGfx;
	uint32_t vertexStride = 0;
	uint32_t vertexCount = 0;
	uint32_t vertexOffset = 0;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
};

class IndexBuffer
{
public:
	IndexBuffer() = default;
	uint32_t GetIndexCount() const { return indexCount; }
private:
	friend LowLvlGfx;
	uint32_t indexCount = 0;
	uint32_t startIndexLocation = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
};


class Texture2D
{
	friend LowLvlGfx;
	friend DX11;
public:
	Texture2D() = default;
	~Texture2D() = default;
	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;
private:

	bool fixedRes = true;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
};


class Sampler
{
	friend LowLvlGfx;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;
};