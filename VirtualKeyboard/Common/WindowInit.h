#pragma once
#include "../Common/Common.h"
#include<iostream>
#include<string>

class WindowInit
{
private:
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	int m_clientWidth = 480, m_clientHeight = 512;
	std::wstring m_windowCaption = L"Virtual Keyboard";

	bool m_pause;
public:
	bool InitMainWindow(HINSTANCE hinstance);
	LRESULT static CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	/// <summary>
	/// Get/Set
	/// </summary>
public:
	HINSTANCE GetInstance() const { return m_hInstance; }
	HWND GetHwnd() const { return m_hwnd; }
};

