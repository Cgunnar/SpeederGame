#include "pch.hpp"
#include "ShipControllerGamePad.h"
#include "ShipScript.h"
#include "Input.h"
using namespace rfe;
using namespace rfm;

void ShipControllerGamePad::OnStart()
{

}

void ShipControllerGamePad::OnUpdate(float dt)
{
	ShipScript::ShipControls& shipController = GetComponent<ShipScript>()->shipController;
	auto gPad = Input::Get().GamePadState();
	auto gPadOld = Input::Get().OldGamePadState();
	if (gPad.IsConnected())
	{
		shipController.reset = gPad.IsBPressed() && !gPadOld.IsBPressed();
		shipController.dockToggle = gPad.IsAPressed() && !gPadOld.IsAPressed();

		shipController.pitchInput = -gPad.thumbSticks.leftY;
		shipController.rollInput = gPad.thumbSticks.leftX;

		if (gPad.IsLeftShoulderPressed())
			shipController.yawInput = 1;
		else if (gPad.IsRightShoulderPressed())
			shipController.yawInput = -1;
		else
			shipController.yawInput = 0;

		shipController.mainThrottle = gPad.triggers.right;
		shipController.secondaryThrottle = gPad.triggers.left;

		shipController.cameraPitch += gPad.thumbSticks.rightY * dt;
		shipController.cameraYaw += gPad.thumbSticks.rightX * dt;
		shipController.cameraPitch = std::clamp(shipController.cameraPitch, -PIDIV2, PIDIV2);
		shipController.cameraYaw = fmod(shipController.cameraYaw, PI2);

		shipController.fireMainWeapon = gPad.IsXPressed();
	}
}
