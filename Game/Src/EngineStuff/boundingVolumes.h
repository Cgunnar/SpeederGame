#pragma once

#include "RimfrostMath.hpp"

struct AABB
{
	//rfm::Vector3 widthHeightDepth;
	rfm::Vector3 min;
	rfm::Vector3 max;
	static AABB Merge(AABB a, AABB b);
	rfm::Vector3 GetWidthHeightDepth() const;
	std::array<rfm::Vector3, 8> GetPointsTransformed(rfm::Transform m = rfm::Transform()) const;
};
AABB operator*(rfm::Matrix m, AABB aabb);

struct Plane
{
	rfm::Vector3 point;
	rfm::Vector3 normal;
};