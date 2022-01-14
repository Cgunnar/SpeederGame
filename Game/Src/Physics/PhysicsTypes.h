#pragma once
#include "RimfrostMath.hpp"
struct sumP
{
	float n = 0;
	float t = 0;
	float b = 0;
};
struct CollisionPoint
{
	rfm::Vector3 intersectionPoint;
	rfm::Vector3 pointRealPosition;
	float penetration;
	rfm::Vector3 normal;
};

struct ConstraintInfo
{
	CollisionPoint cp;
	rfm::Vector3 tangent;
	rfm::Vector3 biTangent;
	sumP sumP;
};