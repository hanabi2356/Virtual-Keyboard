#pragma once
#include "../VirtualKeyboard/CheonjiinEngine.h"
class WindowInit;
class DirectxInit;

class Application
{
private:
	WindowInit* m_window = nullptr;
	DirectxInit* m_dx = nullptr;
	CheonjiinEngine m_ime;

public:

	/// <summary>
	/// 창과 디바이스의 주소를 저장한다
	/// 둘 다 Run보다 오래 생존해야함
	/// </summary>
	/// <param name="window"></param>
	/// <param name="dx"></param>
	explicit Application(WindowInit& window, DirectxInit& dx);

	/// <summary>
	/// 파생 클래스를 Application 포인터로 지울 수 있게 한다
	/// </summary>
	virtual ~Application() = default;

	/// <summary>
	/// 메시지를 비운 뒤, 창이 활성화되어 있을 때 타이머와 Update, Render를 반복한다
	/// 일시정지에서 돌아오면 타이머 기준점을 다시 맞춘다
	/// </summary>
	/// <returns></returns>
	int Run();

protected:
	virtual void Update(float dt);

/// <summary>
/// get/set
/// </summary>
protected:
	DirectxInit* GetDx()const { return m_dx; }
};
