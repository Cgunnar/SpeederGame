#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
struct TransformComp : rfe::Component<TransformComp>
{
	rfm::Transform transform;
	operator rfm::Transform& () { return transform; }
	operator rfm::Matrix& () { return transform; }
	operator const rfm::Transform& () const { return transform; }
	operator const rfm::Matrix& () const { return transform; }
};