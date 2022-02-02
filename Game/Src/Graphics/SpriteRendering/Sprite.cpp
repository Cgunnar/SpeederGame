#include "pch.hpp"
#include "Sprite.h"

using namespace DirectX;
Sprite::Sprite(std::shared_ptr<Texture2D> texture, rfm::Vector2 pos, rfm::Vector2 scale, int layer) : m_texture(texture), m_position(pos), m_scale(scale), m_layer(layer)
{
	D3D11_TEXTURE2D_DESC desc;
	m_texture->buffer->GetDesc(&desc);
	m_rect.bottom = desc.Height;
	m_rect.right = desc.Width;
}

void Sprite::Draw(DirectX::SpriteBatch& batch, Resolution res)
{
	float halfWidth = static_cast<float>(res.width / 2);
	float halfHeight = static_cast<float>(res.height / 2);
	float scaleFactor = (2.0f * halfWidth) * (2.0f * halfHeight) / (1920.0f * 1080.0f);
	scaleFactor = sqrt(scaleFactor);
	float sx = m_scale.x * scaleFactor;
	float sy = m_scale.y * scaleFactor;
	batch.Draw(m_texture->srv.Get(), XMFLOAT2(halfWidth * m_position.x, -halfHeight * m_position.y), &m_rect, Colors::White,
		0, XMFLOAT2(-halfWidth / sx, -halfHeight / sy), XMFLOAT2(sx, sy), SpriteEffects_None, static_cast<float>(m_layer));
}
