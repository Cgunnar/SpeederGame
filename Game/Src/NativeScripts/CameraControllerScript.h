#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class CameraControllerScript : public rfe::NativeScriptComponent<CameraControllerScript>
{
public:
	void OnUpdate(float dt);


private:



	float m_moveSpeed = 1;
};