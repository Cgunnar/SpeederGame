#pragma once
#include "NativeScript.h"



class ShipControllerGamePad : public rfe::NativeScriptComponent<ShipControllerGamePad>
{
public:
	ShipControllerGamePad() = default;
	~ShipControllerGamePad() = default;
	void OnStart();
	void OnUpdate(float dt);
};

