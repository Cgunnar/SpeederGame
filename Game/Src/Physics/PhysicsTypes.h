#pragma once
#include "RimfrostMath.hpp"

struct CollisionPoint
{
	rfm::Vector3 intersectionPoint;
	rfm::Vector3 pointRealPosition;
	float penetration;
	rfm::Vector3 normal;
	operator bool() { return normal.length() > 0.0f; }
};

struct ConstraintInfo
{
	rfm::Vector3 tangent;
	float pen = 0;
	rfm::Vector3 normal;
	float sumPn = 0;
	rfm::Vector3 r;
	float sumPt = 0;
};