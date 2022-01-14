#pragma once
#include "NativeScript.h"

class BoxPhysicsTestScript : public rfe::NativeScriptComponent<BoxPhysicsTestScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);
	void OnFixedUpdate(float dt);
private:
	float m_friction = 0.7f;
};
