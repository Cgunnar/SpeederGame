#pragma once
#include "NativeScript.h"

class TerrainScript : public rfe::NativeScriptComponent<TerrainScript>
{
public:
	void OnStart();
	void OnUpdate(float dt);

private:
};

