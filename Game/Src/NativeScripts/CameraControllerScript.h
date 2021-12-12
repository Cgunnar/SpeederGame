#pragma once
#include "rfEntity.hpp"
#include "RimfrostMath.hpp"

class CameraControllerScript : public rfe::NativeScriptComponent<CameraControllerScript>
{
public:
	void OnUpdate(float dt);
private:
	float m_moveSpeed = 1;
	float m_pitch = 0;
	float m_yaw = 0;
};