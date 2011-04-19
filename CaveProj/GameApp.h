#pragma once
#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_

#include "Application.h"
#include "ScreenSystem.h"

class GameApp : public Application
{
public:
	GameApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);

private:
	ScreenSystem _screenSystem;
};

#endif