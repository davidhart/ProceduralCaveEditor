#pragma once
#ifndef _GAMESCREEN_H_
#define _GAMESCREEN_H_

#include "ScreenSystem.h"

#include "Environment.h"
#include "Player.h"
#include "ChestCounter.h"

class GameScreen : public Screen
{
public:
	GameScreen(const std::wstring& level);
	void Load(RenderWindow& renderWindow);
	void Unload(RenderWindow& renderWindow);
	void Draw(RenderWindow& renderWindow);
	void Update(float dt, RenderWindow& renderWindow);

private:
	Player _player;
	Environment _environment;
	ChestCounter _chestCount;
	std::wstring _level;
	ID3DX10Font* _bigFont;
	ID3DX10Font* _smallFont;
};

#endif