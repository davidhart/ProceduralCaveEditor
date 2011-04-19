#include "Input.h"
#include "RenderWindow.h"
#include <sstream>

Input::Input()
{
	_trapEnabled = false;
	_cursorTrapped = false;
	_hasFocus = false;
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

void Input::TrapCursor(bool trap)
{
	_trapEnabled = trap;

	if (_hasFocus)
	{
		if (_trapEnabled)
			ActivateTrap();
		else
			DeactivateTrap();
	}
}

void Input::SetFocusEvent()
{
	_hasFocus = true;
	if (_trapEnabled)
	{
		ActivateTrap();
	}
}

void Input::LostFocusEvent()
{
	_hasFocus = false;
	if (_trapEnabled)
	{
		DeactivateTrap();
	}
}

void Input::ActivateTrap()
{
	if (!_cursorTrapped)
	{
		_cursorTrapped = true;
		ShowCursor(FALSE);
	}
}

void Input::DeactivateTrap()
{
	if (_cursorTrapped)
	{
		_cursorTrapped = false;
		ShowCursor(TRUE);
	}
}

void Input::PostEvents()
{
	if (_cursorTrapped)
	{
		Vector2f size((float)_window->GetSize().x, (float)_window->GetSize().y);
		_distanceMouseMoved = m_currentMouseState.Position - size / 2;
		RECT r;
		_window->GetScreenRect(r);
		SetCursorPos((r.right + r.left)/2, (r.bottom+r.top)/2);
	}
}