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
	float mass = 1;
	rfm::Vector3 velocity;
	float frictionCof = 0.5f;
	bool resting = false;
};
struct RigidBodyComp : rfe::Component<RigidBodyComp>
{
	RigidBodyComp() = default;
	RigidBodyComp(const RigidBody& rg) : rigidBody(rg){}
	RigidBody rigidBody;
};