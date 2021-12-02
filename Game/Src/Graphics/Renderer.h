#pragma once
#include "GraphicsResources.h"
class Renderer
{
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void Render();

private:
	ConstantBuffer m_worldMatrixCBuffer;
};

