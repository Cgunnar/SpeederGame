#include "pch.hpp"
#include "SkyBox.h"

#pragma warning(push, 0)
#include "stb_image.h"
#pragma warning(pop)

void SkyBox::Init(const std::string& path)
{
	assert(path.substr(path.length() - 4, 4) == ".hdr");

	float* hdr
}
