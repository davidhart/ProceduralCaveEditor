#include "Input.h"
#include "RenderWindow.h"
#include <sstream>

Input::Input()
{
	ZeroMemory(m_currentMouseState.Buttons, sizeof(m_currentMouseState.Buttons));
	ZeroMemory(m_prevMouseState.Buttons, sizeof(m_prevMouseState.Buttons));

	ZeroMemory(m_currentKeyState, sizeof(m_currentKeyState));
	ZeroMemory(m_prevKeyState, sizeof(m_prevKeyState));
}

void Input::UpdateStep()
{
	m_prevMouseState = m_currentMouseState;
	memcpy_s(m_prevKeyState, sizeof(KeyboardState), m_currentKeyState, sizeof(KeyboardState));
}

void Input::MouseMoveEvent(const Vector2f& position)
{
	m_currentMouseState.Position = position;
}

void Input::MouseDownEvent(eButton button)
{
	m_currentMouseState.Buttons[button] = true;
}

void Input::MouseUpEvent(eButton button)
{
	m_currentMouseState.Buttons[button] = false;
}

void Input::KeyDownEvent(eKey key)
{
	m_currentKeyState[key] = true;
}
void Input::KeyUpEvent(eKey key)
{
	m_currentKeyState[key] = false;
}