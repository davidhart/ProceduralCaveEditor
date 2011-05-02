#pragma once
#ifndef _MENUSCREEN_H_
#define _MENUSCREEN_H_

#include "ScreenSystem.h"
#include <string>
#include <vector>

#include <D3DX10.h>

class MenuScreen : public Screen
{
public:
	MenuScreen();
	void Load(RenderWindow& renderWindow);
	void Unload(RenderWindow& renderWindow);
	void Draw(RenderWindow& renderWindow);
	void Update(float dt, RenderWindow& renderWindow);

	void SetMainMenuItems();
	void SetLevelMenuItems();

	void SelectQuit();
	void SelectLevel1();
	void SelectLevel2();
	void SelectLevel3();
	void SelectLevel4();

private:
	class MenuItem
	{
	private:
		typedef void (MenuScreen::*Callback)();

	public:
		MenuItem(const std::string& name, Callback cb);

		Callback Cb;
		std::string Name;
		bool Hovered;
	};

	std::vector<MenuItem> _menuItems;
	ID3DX10Font* _font;
};

#endif