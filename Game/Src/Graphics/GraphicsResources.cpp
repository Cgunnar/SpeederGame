#include "pch.hpp"
#include "GraphicsResources.h"
#include "LowLvlGfx.h"

namespace standardDescriptors
{
	const D3D11_SAMPLER_DESC g_sample_linear_wrap = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
	const D3D11_SAMPLER_DESC g_sample_linear_clamp = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_ALWAYS,
		.BorderColor = {0,0,0,0},
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};
}

SubMesh::SubMesh(const std::vector<Vertex_POS_NOR_UV>& vertices, const std::vector<uint32_t>& indices)
{
	ib = LowLvlGfx::CreateIndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));
	vb = LowLvlGfx::CreateVertexBuffer(reinterpret_cast<const float*>(vertices.data()), vertices.size() * sizeof(Vertex_POS_NOR_UV), sizeof(Vertex_POS_NOR_UV));
	this->indexCount = indices.size();
}

SubMesh::SubMesh(const std::vector<Vertex_POS_NOR_UV_TAN_BITAN>& vertices, const std::vector<uint32_t>& indices)
{
	ib = LowLvlGfx::CreateIndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));
	vb = LowLvlGfx::CreateVertexBuffer(reinterpret_cast<const float*>(vertices.data()), vertices.size() * sizeof(Vertex_POS_NOR_UV_TAN_BITAN), sizeof(Vertex_POS_NOR_UV_TAN_BITAN));
	this->indexCount = indices.size();
}
