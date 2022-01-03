#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
#include <vector>

struct TransformComp : rfe::Component<TransformComp>
{
	rfm::Transform transform;
	operator rfm::Transform& () { return transform; }
	operator rfm::Matrix& () { return transform; }
	operator const rfm::Transform& () const { return transform; }
	operator const rfm::Matrix& () const { return transform; }
};

struct PlayerComp : rfe::Component<PlayerComp>
{
	
};

struct ChildComp : rfe::Component<ChildComp>
{
	std::vector<rfe::Entity> children;
};