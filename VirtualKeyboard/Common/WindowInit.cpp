#include "WindowInit.h"

/// <summary>
/// 창 클래스를 등록하고 클라이언트 크기 480x512로 메인 윈도우를 만든다.
/// 마지막 인자 this는 WM_NCCREATE에서 인스턴스를 찾는 데 쓴다.
/// </summary>
bool WindowInit::InitMainWindow(HINSTANCE hinstance)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc)) return false;

    RECT r = { 0, 0, m_clientWidth, m_clientHeight };
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
    int width = r.right - r.left;
    int height = r.bottom - r.top;

    m_hwnd = CreateWindow(wc.lpszClassName, m_windowCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hinstance, this);

    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

/// <summary>
/// GWLP_USERDATA의 WindowInit으로 창 메시지를 처리한다.
/// WM_SIZE의 lParam 하위 워드가 클라이언트 너비, 상위 워드가 높이다.
/// </summary>
LRESULT WindowInit::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    WindowInit* app = reinterpret_cast<WindowInit*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (app)
    {
        switch (msg)
        {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                app->m_pause = true;
            }
            else
            {
                app->m_pause = false;
            }
            return 0;
        case WM_SIZE:
            app->m_clientWidth = LOWORD(lParam);
            app->m_clientHeight = HIWORD(lParam);
            if (wParam == SIZE_MINIMIZED)
            {
                app->m_pause = true;
                app->m_minimized = true;
                app->m_maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                app->m_pause = false;
                app->m_minimized = false;
                app->m_maximized = true;
                //Resize
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (app->m_minimized)
                {
                    app->m_pause = false;
                    app->m_minimized = false;
                    //Resize
                }
                else if (app->m_maximized)
                {
                    app->m_pause = false;
                    app->m_maximized = false;
                    //Resize
                }
            }
            if (wParam != SIZE_MINIMIZED && app->m_onResize
                && app->m_clientWidth > 0 && app->m_clientHeight > 0)
            {
                app->m_onResize(app->m_clientWidth, app->m_clientHeight,
                    app->m_resizeUser);
            }
            return 0;
        case WM_ENTERSIZEMOVE:

            return 0;
        case WM_EXITSIZEMOVE:
            return 0;
        case WM_LBUTTONDOWN:
            return 0;
        case WM_MOUSEMOVE:
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
