#pragma once
#include "NativeScript.h"

class TerrainScript : public rfe::NativeScriptComponent<TerrainScript>
{
public:
	TerrainScript() = default;
	TerrainScript(uint32_t seed);
	void OnStart();
	void OnUpdate(float dt);

private:
	uint32_t m_seed = 4324345;
};

