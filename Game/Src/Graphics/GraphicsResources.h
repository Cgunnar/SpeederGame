#pragma once

#include <wrl.h>
#include <d3d11.h>
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include "RimfrostMath.hpp"
#include "utilityTypes.h"
#include "GraphicsUtilityTypes.h"
#include "Material.h"

class LowLvlGfx;
class DX11;



namespace standardSamplers
{
	extern const D3D11_SAMPLER_DESC g_linear_wrap;
}

struct alignas(16) PointLight
{
	rfm::Vector3 position{ 0, 0, 0 };
	float lightStrength = 1;
	rfm::Vector3 color = { 1, 1, 1 };
	float constantAttenuation = 1;
	float LinearAttenuation = 0.1f;
	float exponentialAttenuation = 0.1f;
};

class Shader
{
	friend LowLvlGfx;
public:
	Shader() = default;

private:
	Shader(uint32_t id) : m_id(id) {}
	uint32_t m_id = -1;
};

class ConstantBuffer
{
	friend LowLvlGfx;
public:
	ConstantBuffer() = default;
private:
	ConstantBuffer(uint32_t size) : m_size(size) {}
	uint32_t m_size = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

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



struct SubMesh
{
	VertexBuffer vb;
	IndexBuffer ib;
	uint32_t indexCount;
	uint32_t startIndexLocation;
	int32_t baseVertexLocation;
	GID GetGID() const { return guid; }
private:
	GID guid = GID::GenerateNew();
};

typedef size_t RenderUnitID;

struct RenderUnit
{
	SubMesh subMesh;
	Material material;
};

struct SubModel
{
	std::vector<SubModel> subModels;
	std::vector<RenderUnitID> renderUnitIDs;
	RenderUnitID RenderUnitBegin, RenderUnitEnd;	// begin is renderUnitIDs[0], end is the last RenderUnitID+1 in the the tree of submodels
													// this is usefull if we want to give a function or class the ability to
													// loop over all Id:s but dont want to save the tree of submodels
};

struct Model : public SubModel
{
	
	VertexBuffer vb;
	IndexBuffer ib;
};




