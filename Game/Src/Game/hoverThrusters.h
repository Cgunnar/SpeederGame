#pragma once
#include "RimfrostMath.hpp"
class HoverThrusters
{
public:
	rfm::Vector3 CalculateForce(float power);
	float distanceOverGround;
private:
};

