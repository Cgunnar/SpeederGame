#pragma once
#include "NativeScript.h"



class ShipControllerKBM : public rfe::NativeScriptComponent<ShipControllerKBM>
{
public:
	ShipControllerKBM() = default;
	~ShipControllerKBM() = default;
	void OnStart();
	void OnUpdate(float dt);
};

