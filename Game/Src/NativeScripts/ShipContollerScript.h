#pragma once
#include "NativeScript.h"
#include "RimfrostMath.hpp"

class ShipContollerScript : public rfe::NativeScriptComponent<ShipContollerScript>
{
public:
	void OnUpdate(float dt);
private:
	void reset();
	float m_yawSpeed = rfm::DegToRad(20);
	float m_pitchSpeed = rfm::DegToRad(40);
	float m_rollSpeed = rfm::DegToRad(50);
	float m_thrustSpeed = 3;
	bool m_docked = true;
};


class ShipScript : public rfe::NativeScriptComponent<ShipScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
private:
};
