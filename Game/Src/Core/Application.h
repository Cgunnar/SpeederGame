#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"
#include "PhysicsEngine.h"
//#include "AssetManager.h

class Application
{
public:
	Application();
	~Application();
	Application(const Application& other) = delete;
	Application& operator=(const Application& other) = delete;
	void Run();
private:
	Window* m_window = nullptr;
	Renderer* m_renderer = nullptr;
	Scene* m_scene = nullptr;
	PhysicsEngine m_physicsEngine = PhysicsEngine(1.0 / 120.0);
};

