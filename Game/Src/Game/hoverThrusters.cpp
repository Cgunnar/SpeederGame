#include "pch.hpp"
#include "hoverThrusters.h"
#include "PhysicsComponents.h"

using namespace rfm;
using namespace rfe;

rfm::Vector3 HoverThrusters::CalculateForce(float power)
{

	return rfm::Vector3(0, power * 0.01f + power / std::max(1.0f, (distanceOverGround * distanceOverGround)), 0);
}
