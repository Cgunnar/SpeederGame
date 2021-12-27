#include "pch.hpp"
#include "ShipContollerScript.h"
#include "Input.h"

void ShipContollerScript::OnUpdate(float dt)
{
	auto gPad = Input::Get().GamePad().GetState(0);

	if (gPad.IsConnected())
	{
		
		auto& transform = getComponent<TransformComp>()->transform;
		transform.rotateDegL(
			dt * gPad.thumbSticks.leftY * m_pitchSpeed,
			dt * gPad.thumbSticks.rightX * m_yawSpeed,
			dt * -gPad.thumbSticks.leftX * m_rollSpeed);

		transform.translateL(0, 0, m_thrustSpeed * dt * gPad.triggers.right);

		if (gPad.IsAPressed()) Input::Get().GamePad().SetVibration(0, 1, 1);
		else Input::Get().GamePad().SetVibration(0, 0, 0);
	}
}
