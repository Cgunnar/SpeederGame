#pragma once

#include "RimfrostMath.hpp"

struct AABB
{
	//rfm::Vector3 widthHeightDepth;
	rfm::Vector3 min;
	rfm::Vector3 max;
	static AABB Merge(AABB a, AABB b);
};