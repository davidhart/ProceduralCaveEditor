#pragma once

#include "Vector.h"
class RenderWindow;

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
		KEY_ESC = 0x1B,

		KEY_A = 0x41,
		KEY_D = 0x44,
		KEY_E = 0x45,
		KEY_F = 0x46,
		KEY_G = 0x47,
		KEY_H = 0x48,

		KEY_S = 0x53,
		
		KEY_W = 0x57,
	};

	friend class RenderWindow;

	inline const Vector2f& GetCursorPosition() const { return m_currentMouseState.Position; }
	inline const Vector2f GetMouseDistance() const 
	{ 
		if(_trapEnabled) 
		{ 
			if (_cursorTrapped) 
				return _distanceMouseMoved; 
			else 
				return Vector2f(0, 0);
		}
		else  
		{ 
			return m_currentMouseState.Position - m_prevMouseState.Position; 
		}
	}

	inline bool IsButtonDown(eButton button) const { return m_currentMouseState.Buttons[button]; }
	inline bool IsButtonJustPressed(eButton button) const { return m_currentMouseState.Buttons[button] && !m_prevMouseState.Buttons[button]; }
	inline bool IsButtonJustReleased(eButton button) const  { return !m_currentMouseState.Buttons[button] && m_prevMouseState.Buttons[button]; }

	inline bool IsKeyDown(eKey key) const { return m_currentKeyState[key]; }
	inline bool IsKeyJustPressed(eKey key) const { return m_currentKeyState[key] && !m_prevKeyState[key]; }

	void TrapCursor(bool trap);

private:
	Input();
	void UpdateStep();

	void MouseMoveEvent(const Vector2f& position);
	void MouseDownEvent(eButton button);
	void MouseUpEvent(eButton button);
	void KeyDownEvent(eKey key);
	void KeyUpEvent(eKey key);
	void PostEvents();

	void LostFocusEvent();
	void SetFocusEvent();

	void ActivateTrap();
	void DeactivateTrap();

	inline void SetWindow(RenderWindow* window) { _window = window; }

	bool _cursorTrapped;
	bool _trapEnabled;
	bool _hasFocus;

	Vector2f _distanceMouseMoved;

	RenderWindow* _window;

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