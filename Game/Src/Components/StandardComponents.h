#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"
#include <vector>

struct TransformComp : rfe::Component<TransformComp>
{
	TransformComp() = default;
	TransformComp(rfm::Vector3 position)
	{
		transform.setTranslation(position);
	}
	TransformComp(rfm::Vector3 position, rfm::Vector3 rotation)
	{
		transform.setTranslation(position);
		transform.setRotation(rotation);
	}
	TransformComp(rfm::Vector3 position, rfm::Vector3 rotation, rfm::Vector3 scale)
	{
		transform.setTranslation(position);
		transform.setRotation(rotation);
		transform.setScale(scale);
	}
	rfm::Transform transform;
	operator rfm::Transform& () { return transform; }
	operator rfm::Matrix& () { return transform; }
	operator const rfm::Transform& () const { return transform; }
	operator const rfm::Matrix& () const { return transform; }
};

struct PlayerComp : rfe::Component<PlayerComp>
{
	
};

struct CameraComp : rfe::Component<CameraComp>
{

};

struct ChildComp : rfe::Component<ChildComp>
{
	std::vector<rfe::Entity> children;
};