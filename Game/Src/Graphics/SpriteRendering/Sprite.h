#pragma once

#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "GraphicsResources.h"
#include "RimfrostMath.hpp"

class Sprite
{
public:
	Sprite() = default;
	Sprite(std::shared_ptr<Texture2D> texture, rfm::Vector2 pos, rfm::Vector2 scale = { 1,1 }, int layer = 0);
	bool visible = false;
	void Draw(DirectX::SpriteBatch& batch, Resolution res);
private:
	std::shared_ptr<Texture2D> m_texture = nullptr;
	RECT m_rect{};
	rfm::Vector2 m_position;
	rfm::Vector2 m_scale;
	int m_layer;
};

