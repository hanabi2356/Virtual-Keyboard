#pragma once
#include "../Common/Common.h"
#include<iostream>
#include<string>

using ResizeCallback = void(*)(int width, int height, void* user);

class WindowInit
{
private:
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	int m_clientWidth = 480, m_clientHeight = 512;
	std::wstring m_windowCaption = L"Virtual Keyboard";

	bool m_pause = false;
	bool m_maximized = false;
	bool m_minimized = false;

	ResizeCallback m_onResize = nullptr;
	void* m_resizeUser = nullptr;
public:
	bool InitMainWindow(HINSTANCE hinstance);
	LRESULT static CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	/// <summary>
	/// Get/Set
	/// </summary>
public:
	HINSTANCE GetInstance() const { return m_hInstance; }
	HWND GetHwnd() const { return m_hwnd; }
	bool IsPause()const { return m_pause; }
	void SetResizeCallback(ResizeCallback cb, void* user)
	{
		m_onResize = cb;
		m_resizeUser = user;
	}
};

