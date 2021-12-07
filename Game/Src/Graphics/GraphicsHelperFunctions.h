#pragma once


namespace GfxHelpers
{
	void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride);
}

