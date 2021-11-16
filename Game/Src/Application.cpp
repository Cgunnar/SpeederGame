#include "Application.h"

Application::Application()
{
	m_window = new Window();
}

Application::~Application()
{
	delete m_window;
}

void Application::Run()
{
	bool running = true;
	while (running)
	{
		running = m_window->Win32MsgPump();
		if (!running)
		{
			break;
		}
	}
}
