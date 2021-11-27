#pragma once
#include <string>
#include <Windows.h>
class LowLvlGfx;
class Shader
{
	friend LowLvlGfx;
public:

private:
	Shader(uint32_t id) : m_id(id) {}
	uint32_t m_id;
};

