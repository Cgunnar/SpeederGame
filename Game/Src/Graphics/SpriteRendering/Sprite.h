#pragma once
#include "GraphicsResources.h"

class Sprite
{
public:
	Sprite() = default;
	Sprite(std::shared_ptr<Texture2D> texture);
private:
	std::shared_ptr<Texture2D> m_texture = nullptr;
};

