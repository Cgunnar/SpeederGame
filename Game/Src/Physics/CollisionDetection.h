#pragma once
#include "RimfrostMath.hpp"
#include "boundingVolumes.h"
#include "PhysicsTypes.h"



namespace colDetect
{
	std::vector<CollisionPoint> PlaneVSPoints(Plane plane, std::vector<rfm::Vector3> points);
	CollisionPoint PlaneVSPoint(Plane plane, rfm::Vector3 point);

}

