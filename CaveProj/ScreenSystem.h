#pragma once
#ifndef _SCREENSYSTEM_H_
#define _SCREENSYSTEM_H_

class Input;
class RenderWindow;
class ScreenSystem;

class Screen
{
public:
	friend ScreenSystem;

	virtual void Load(RenderWindow& renderWindow) = 0;
	virtual void Unload(RenderWindow& renderWindow) = 0;
	virtual void Draw(RenderWindow& renderWindow) = 0;
	virtual void Update(float dt, RenderWindow& renderWindow) = 0;

protected:
	ScreenSystem* _parent;
};

class ScreenSystem
{
public:
	ScreenSystem(RenderWindow& renderWindow);
	~ScreenSystem();
	void NewScreen(Screen* screen);
	void Load();
	void Unload();
	void Draw();
	void Update(float dt);
	inline Screen* CurrentScreen() const { return _currentScreen; }

private:
	Screen* _currentScreen;
	bool _loaded;
	RenderWindow& _renderWindow;
};

#endif