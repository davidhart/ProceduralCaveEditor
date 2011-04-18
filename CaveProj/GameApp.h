#pragma once
#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_

#include "Application.h"
#include "Environment.h"
#include "Player.h"

class GameApp : public Application
{
public:
	GameApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);

private:
	Player _player;
	Environment _environment;
};

#endif