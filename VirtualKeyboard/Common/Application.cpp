#include "Application.h"
#include "WindowInit.h"
#include "DirectxInit.h"
#include "GameTimer.h"
Application::Application(WindowInit& window, DirectxInit& dx)
	:m_window(&window), m_dx(&dx)
{
}

int Application::Run()
{
	GameTimer* timer = GameTimer::GetInstance();
	timer->Initialize();
	bool resumed = false;

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) return static_cast<int>(msg.wParam);
		}

		if (m_window->IsPause())
		{
			resumed = true;
			Sleep(10);
			continue;
		}
		if (resumed)
		{
			timer->Initialize();
			resumed = false;
		}
		timer->Update();
		Update(timer->GetDeltaTime());
		m_dx->Render();
	}
	return static_cast<int>(msg.wParam);
}

void Application::Update(float dt)
{
	
}
