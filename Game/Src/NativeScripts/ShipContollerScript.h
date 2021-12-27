#pragma once
#include "NativeScript.h"

class ShipContollerScript : public rfe::NativeScriptComponent<ShipContollerScript>
{
public:
	void OnUpdate(float dt);
private:
	float m_yawSpeed = 20;
	float m_pitchSpeed = 40;
	float m_rollSpeed = 50;
	float m_thrustSpeed = 3;
};

