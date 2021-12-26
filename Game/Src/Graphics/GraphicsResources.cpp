#include "pch.hpp"
#include "GraphicsResources.h"

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