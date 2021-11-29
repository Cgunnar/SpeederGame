#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
struct TransformComp : rfe::Component<TransformComp>
{
	rf::Transform transform;
	operator rf::Transform& () { return transform; }
	operator const rf::Transform& () const { return transform; }
};