#pragma once

#include "Vector.h"

class Input
{
public:
	enum eButton
	{
		BUTTON_LEFT = 0,
		BUTTON_MID = 1,
		BUTTON_RIGHT = 2, 
	};

	enum eKey
	{
		KEY_SPACE = 0x20,
		KEY_W = 0x57,
		KEY_A = 0x41,
		KEY_S = 0x53,
		KEY_D = 0x44,
	};

	friend class RenderWindow;

	inline const Vector2f& GetCursorPosition() const { return m_currentMouseState.Position; }
	inline const Vector2f GetMouseDistance() const { return m_currentMouseState.Position - m_prevMouseState.Position; }

	inline bool IsButtonDown(eButton button) const { return m_currentMouseState.Buttons[button]; }
	inline bool IsButtonJustPressed(eButton button) const { return m_currentMouseState.Buttons[button] && !m_prevMouseState.Buttons[button]; }

	inline bool IsKeyDown(eKey key) const { return m_currentKeyState[key]; }
	inline bool IsKeyJustPressed(eKey key) const { return m_currentKeyState[key] && !m_prevKeyState[key]; }

private:
	Input();
	void UpdateStep();

	void MouseMoveEvent(const Vector2f& position);
	void MouseDownEvent(eButton button);
	void MouseUpEvent(eButton button);
	void KeyDownEvent(eKey key);
	void KeyUpEvent(eKey key);

	struct MouseState
	{
		Vector2f Position;
		bool Buttons[3];
	};

	MouseState m_prevMouseState;
	MouseState m_currentMouseState;

	typedef bool KeyboardState[256];

	KeyboardState m_currentKeyState;
	KeyboardState m_prevKeyState;
};