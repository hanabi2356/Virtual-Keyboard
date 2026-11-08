#pragma once

#include "../Common/Common.h"
#include "../Common/MonoSingleton.h"
class InputManager : public MonoSingleton<InputManager>
{
	friend class MonoSingleton<InputManager>;

private:
	InputManager() = default;
	~InputManager() override = default;

	int m_x = 0;
	int m_y = 0;
	bool m_hasClick = false;
	bool m_down = false;

public:

	/// <summary>
	/// 왼쪽 버튼을 누른 클라이언트 좌표를 저장한다.
	/// </summary>
	void OnLeftDown(int x, int y);

	/// <summary>
	/// 왼쪽 버튼에서 손을 뗀 상태로 바꾼다. 아직 읽지 않은 클릭 좌표는 유지한다.
	/// </summary>
	void OnLeftUp();

	/// <summary>
	/// 아직 처리하지 않은 클릭이 있으면 좌표를 넘기고 그 클릭을 지운다.
	/// </summary>
	bool ConsumeClick(int& x, int& y);

	int GetX() const { return m_x; }
	int GetY() const { return m_y; }
	bool HasClick() const { return m_hasClick; }
	bool IsDown() const { return m_down; }
};

