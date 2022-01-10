#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
#include "boundingVolumes.h"

struct AABBComp : rfe::Component<AABBComp>
{
	AABB aabb;
};

struct RigidBody
{
	rfm::Matrix3 momentOfInertia;
	rfm::Vector3 angularVelocity;
	float mass;
	rfm::Vector3 velocity;
};
struct RigidBodyComp : rfe::Component<RigidBodyComp>
{
	RigidBody rigidBody;
};