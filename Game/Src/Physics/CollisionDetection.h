#pragma once
#include "RimfrostMath.hpp"
#include "boundingVolumes.h"

struct CollisionPoint
{
	rfm::Vector3 intersectionPoint;
	rfm::Vector3 pointRealPosition;
	float penetration;
	rfm::Vector3 normal;
};

namespace colDetect
{
	std::vector<CollisionPoint> PlaneVSPoints(Plane plane, std::vector<rfm::Vector3> points);

}

