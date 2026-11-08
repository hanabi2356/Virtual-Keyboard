#include "../Common/WindowInit.h"
#include "../Common/DirectxInit.h"
#include "../Common/Application.h"
#include<fstream>
#include<sstream>
#include<string>



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    WindowInit window;
    DirectxInit dx(&window);

    if (window.InitMainWindow(hInstance) == false) return 0;
    if (dx.DXInit() == false) return 0;
   
    if (dx.CreatePSO(L"../Shader/TextureQuad.hlsl") == false) return 0;
    if (dx.CreateKeyboard() == false) return 0;

    Application app(window, dx);
    return app.Run();
}

