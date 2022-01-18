#pragma once
#include "RimfrostMath.hpp"
#include "boundingVolumes.h"
#include "PhysicsTypes.h"



namespace colDetect
{
	std::vector<CollisionPoint> PlaneVSPoints(rfm::Plane plane, std::vector<rfm::Vector3> points);
	CollisionPoint PlaneVSPoint(rfm::Plane plane, rfm::Vector3 point);

}

