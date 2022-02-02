#pragma once
#include <SpriteBatch.h>
#include <SpriteFont.h>

#include "Sprite.h"

class SpriteRendere
{
public:
	SpriteRendere() = default;
	void Init();
	
	void Draw(std::vector<Sprite> sprites);
private:
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
};

