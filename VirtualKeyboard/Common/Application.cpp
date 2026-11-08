#include "Application.h"
#include "WindowInit.h"
#include "DirectxInit.h"
#include "GameTimer.h"
#include "InputManager.h"
#include "../VirtualKeyboard/KeyboardImage.h"
Application::Application(WindowInit& window, DirectxInit& dx)
	:m_window(&window), m_dx(&dx)
{
}

int Application::Run()
{
	GameTimer* timer = GameTimer::GetInstance();
	timer->Initialize();
	bool resumed = false;
	if (!CheonjiinEngine::SelfCheck())
		OutputDebugStringW(L"CheonjiinEngine::SelfCheck failed\n");

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

/// <summary>
/// 클릭된 키를 천지인 조합에 넣고 결과를 위 라벨에 표시한다.
/// </summary>
void Application::Update(float dt)
{
	m_ime.Tick(dt);
	int x = 0;
	int y = 0;
	if (!InputManager::GetInstance()->ConsumeClick(x, y)) return;

	RECT rc = {};
	GetClientRect(m_window->GetHwnd(), &rc);
	CheonjiinKeyId key = HitKey(rc.right - rc.left, rc.bottom - rc.top, x, y);
	if (key == CheonjiinKeyId::None) return;
	m_ime.Press(key);
	m_dx->SetKeyboardLabel(m_ime.Text());
}
