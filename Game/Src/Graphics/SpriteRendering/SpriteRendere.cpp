#include "pch.hpp"
#include "SpriteRendere.h"
#include "LowLvlGfx.h"

using namespace DirectX;
void SpriteRendere::Init()
{
	m_spriteBatch = std::make_unique<SpriteBatch>(LowLvlGfx::Context().Get());
}

void SpriteRendere::Draw(std::vector<Sprite> sprites)
{
	Resolution res = LowLvlGfx::GetResolution();
	m_spriteBatch->Begin();

	for (auto& s : sprites)
	{
		s.Draw(*m_spriteBatch, res);
	}
	m_spriteBatch->End();
	LowLvlGfx::Context()->OMSetDepthStencilState(nullptr, 0);
}
