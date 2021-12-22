#include "pch.hpp"
#include "GraphicsResources.h"

namespace standardDescriptors
{
	const D3D11_SAMPLER_DESC g_linear_wrap = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f,
		1,
		D3D11_COMPARISON_ALWAYS,
		{0,0,0,0},
		0,
		D3D11_FLOAT32_MAX
	};
}