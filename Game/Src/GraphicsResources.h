#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <windows.h>
#include <cstdint>

class LowLvlGfx;
class DX11;

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