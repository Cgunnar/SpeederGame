#pragma once
#include "NativeScript.h"
#include "RimfrostMath.hpp"

class ShipContollerScript : public rfe::NativeScriptComponent<ShipContollerScript>
{
public:
	void OnUpdate(float dt);
	bool m_docked = true;
private:
	void reset();
	float m_yawSpeed = rfm::DegToRad(20);
	float m_pitchSpeed = rfm::DegToRad(80);
	float m_rollSpeed = rfm::DegToRad(50);
	float m_thrustSpeed = 14;
};


class ShipScript : public rfe::NativeScriptComponent<ShipScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
	void OnFixedUpdate(float dt);
private:
	float m_friction = 0.7f;
};
