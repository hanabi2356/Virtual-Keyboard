#pragma once
#include "../Common/Common.h"
#include "../Common/MonoSingleton.h"
class GameTimer : public MonoSingleton<GameTimer>
{
	friend class MonoSingleton<GameTimer>;

private:
	GameTimer();
	~GameTimer()override;

	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_prevCount;

	float m_deltaTime;
	float m_totalTime;

public:
	void Initialize();
	void Update();

	float GetDeltaTime() const { return m_deltaTime; }
	float GetTotalTime() const { return m_totalTime; }
	float GetFPS()const { return (m_deltaTime > 0.0f) ? (1.0f / m_deltaTime) : 0.0f; }
};

