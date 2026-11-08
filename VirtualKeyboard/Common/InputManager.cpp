#include "InputManager.h"

void InputManager::OnLeftDown(int x, int y)
{
    m_x = x;
    m_y = y;
    m_hasClick = true;
    m_down = true;
}

void InputManager::OnLeftUp()
{
    m_down = false;
}

bool InputManager::ConsumeClick(int& x, int& y)
{
    if (!m_hasClick) return false;
    x = m_x;
    y = m_y;
    m_hasClick = false;
    return true;
}


