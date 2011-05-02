#include "MenuScreen.h"
#include "RenderWindow.h"
#include "GameScreen.h"

MenuScreen::MenuScreen()
{
	SetMainMenuItems();
}

void MenuScreen::Load(RenderWindow& renderWindow)
{
	D3DX10_FONT_DESCW fd;
	std::wstring facename(L"Tahoma");
	wcscpy_s(fd.FaceName, facename.size()+1, facename.c_str());
	fd.Width = 0;
	fd.MipLevels = 1;
	fd.CharSet = DEFAULT_CHARSET;
	fd.Height = 48;
	fd.OutputPrecision = OUT_DEFAULT_PRECIS;
	fd.Italic = 0;
	fd.Weight = FW_NORMAL;
	fd.Quality = DEFAULT_QUALITY;
	fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	D3DX10CreateFontIndirectW(renderWindow.GetDevice(), &fd, &_font);
}

void MenuScreen::Unload(RenderWindow& renderWindow)
{
	_font->Release();
}

void MenuScreen::Draw(RenderWindow& renderWindow)
{
	int lineheight = 48;
	int linespace = 15; 
	int height = _menuItems.size() * lineheight + (_menuItems.size() - 1) * linespace;
	RECT pos;
	pos.top = renderWindow.GetSize().y / 2 - (int)height / 2;
	pos.bottom = pos.top + lineheight;
	pos.left = 200;
	pos.right = renderWindow.GetSize().x - 200;

	for (unsigned int i = 0; i < _menuItems.size(); ++i)
	{
		D3DXCOLOR col;
		if (_menuItems[i].Hovered)
			col = D3DXCOLOR(1, 1, 1, 1);
		else
			col = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1);

		_font->DrawTextA(NULL, _menuItems[i].Name.c_str(), -1, &pos, DT_NOCLIP | DT_CENTER | DT_VCENTER, col);
		pos.top += lineheight + linespace;
		pos.bottom += lineheight + linespace;
	}
}

void MenuScreen::Update(float dt, RenderWindow& renderWindow)
{
	const Input& input = renderWindow.GetInput();
	int lineheight = 48;
	int linespace = 15; 
	int height = _menuItems.size() * lineheight + (_menuItems.size() - 1) * linespace;
	RECT pos;
	pos.top = renderWindow.GetSize().y / 2 - (int)height / 2;
	pos.bottom = pos.top + lineheight;
	pos.left = 200;
	pos.right = renderWindow.GetSize().x - 200;

	for (unsigned int i = 0; i < _menuItems.size(); ++i)
	{
		_menuItems[i].Hovered = input.GetCursorPosition().x > pos.left && input.GetCursorPosition().x < pos.right && 
			input.GetCursorPosition().y > pos.top && input.GetCursorPosition().y < pos.bottom;

		if (_menuItems[i].Hovered && input.IsButtonJustPressed(Input::BUTTON_LEFT))
			(this->*_menuItems[i].Cb)();

		pos.top += lineheight + linespace;
		pos.bottom += lineheight + linespace;
	}
}

void MenuScreen::SetMainMenuItems()
{
	_menuItems.clear();

	_menuItems.push_back(MenuItem("Play", &MenuScreen::SetLevelMenuItems));
	_menuItems.push_back(MenuItem("Quit", &MenuScreen::SelectQuit));
}

void MenuScreen::SetLevelMenuItems()
{
	_menuItems.clear();

	_menuItems.push_back(MenuItem("Level 1", &MenuScreen::SelectLevel1));
	_menuItems.push_back(MenuItem("Level 2", &MenuScreen::SelectLevel2));
	_menuItems.push_back(MenuItem("Level 3", &MenuScreen::SelectLevel3));
	_menuItems.push_back(MenuItem("Level 4", &MenuScreen::SelectLevel4));
	_menuItems.push_back(MenuItem("Back", &MenuScreen::SetMainMenuItems));
}

void MenuScreen::SelectQuit()
{
	_parent->NewScreen(NULL);
}

void MenuScreen::SelectLevel1()
{
	_parent->NewScreen(new GameScreen(L"levels/a.cave"));
}

void MenuScreen::SelectLevel2()
{
	_parent->NewScreen(new GameScreen(L"levels/b.cave"));
}

void MenuScreen::SelectLevel3()
{
	_parent->NewScreen(new GameScreen(L"levels/c.cave"));
}

void MenuScreen::SelectLevel4()
{
	_parent->NewScreen(new GameScreen(L"levels/d.cave"));
}

MenuScreen::MenuItem::MenuItem(const std::string& name, MenuScreen::MenuItem::Callback cb) :
	Name(name),
	Cb(cb),
	Hovered(false)
{
}