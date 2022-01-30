#include "pch.hpp"
#include "ShipControllerKBM.h"
#include "ShipScript.h"
#include "Input.h"
using namespace rfe;
using namespace rfm;

void ShipControllerKBM::OnStart()
{

}

void ShipControllerKBM::OnUpdate(float dt)
{
	Input& in = Input::Get();
	ShipScript::ShipControls& shipController = GetComponent<ShipScript>()->shipController;


	if (in.keyBeingPressed(Input::Keys::W))
		shipController.mainThrottle = 1;
	else
		shipController.mainThrottle = 0;

	if (in.keyBeingPressed(Input::Keys::Space))
		shipController.secondaryThrottle = 1;
	else
		shipController.secondaryThrottle = 0;

	if (in.keyBeingPressed(Input::Keys::A) && !in.keyBeingPressed(Input::Keys::D))
		shipController.yawInput = 1;
	else if (!in.keyBeingPressed(Input::Keys::A) && in.keyBeingPressed(Input::Keys::D))
		shipController.yawInput = -1;
	else
		shipController.yawInput = 0;

	shipController.reset = in.keyPressed(Input::Keys::Back);
	shipController.dockToggle = in.keyPressed(Input::Keys::G);

	MouseState ms = in.GetMouse().GetMouseState();
	if (ms.RMBHeld)
	{
		shipController.cameraPitch -= ms.deltaY * ms.mouseCof;
		shipController.cameraYaw += ms.deltaX * ms.mouseCof;
		shipController.cameraPitch = std::clamp(shipController.cameraPitch, -PIDIV2, PIDIV2);
		shipController.cameraYaw = fmod(shipController.cameraYaw, PI2);
	}
	else
	{
		shipController.pitchInput = std::clamp(ms.deltaY, -1.0f, 1.0f);
		shipController.rollInput = std::clamp(ms.deltaX, -1.0f, 1.0f);
	}
		
	shipController.fireMainWeapon = ms.LMBHeld;
}
