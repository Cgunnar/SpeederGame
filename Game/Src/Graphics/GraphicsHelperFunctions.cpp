#include "pch.hpp"
#include "GraphicsHelperFunctions.h"


namespace GfxHelpers
{
	void SetSubResDataMips(const void* dataPtr, D3D11_SUBRESOURCE_DATA*& subResMipArray, int mipNumber, int stride)
	{
		assert(dataPtr);

		subResMipArray = new D3D11_SUBRESOURCE_DATA[mipNumber];
		int SysMemPitch = stride;

		for (int i = 0; i < mipNumber; i++)
		{
			subResMipArray[i].pSysMem = dataPtr;
			subResMipArray[i].SysMemPitch = SysMemPitch;
			subResMipArray[i].SysMemSlicePitch = 0;
			SysMemPitch >>= 1;
		}
	}
}
