#include "GameTimer.h"

GameTimer::GameTimer()
	:m_deltaTime(0.0f), m_totalTime(0.0f)
{
}

GameTimer::~GameTimer()
{
}

void GameTimer::Initialize()
{
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_prevCount);
}

void GameTimer::Update()
{
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    m_deltaTime = static_cast<float>(currentCount.QuadPart - m_prevCount.QuadPart)
        / static_cast<float>(m_frequency.QuadPart);
    m_totalTime += m_deltaTime;
    m_prevCount = currentCount;
    if (m_deltaTime > 0.1f)
        m_deltaTime = 0.1f;
}

