#include "GameScreen.h"

#include "RenderWindow.h"
#include "Input.h"
#include "MenuScreen.h"

GameScreen::GameScreen(const std::wstring& level) :
	_player(_environment),
	_level(level)
{
}

void GameScreen::Load(RenderWindow& renderWindow)
{
	_player.Load(renderWindow);
	_environment.Load(renderWindow.GetDevice(), _player.GetCamera());
	_environment.Open(_level);
	_chestCount.SetTotal(_environment.NumChests());
	_chestCount.Load(renderWindow);
	renderWindow.TrapCursor(true);

	D3DX10_FONT_DESCW fd;
	std::wstring facename(L"Tahoma");
	wcscpy_s(fd.FaceName, facename.size()+1, facename.c_str());
	fd.Width = 0;
	fd.MipLevels = 1;
	fd.CharSet = DEFAULT_CHARSET;
	fd.Height = 72;
	fd.OutputPrecision = OUT_DEFAULT_PRECIS;
	fd.Italic = 0;
	fd.Weight = FW_NORMAL;
	fd.Quality = DEFAULT_QUALITY;
	fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	D3DX10CreateFontIndirectW(renderWindow.GetDevice(), &fd, &_bigFont);

	fd.Height = 36;
	D3DX10CreateFontIndirectW(renderWindow.GetDevice(), &fd, &_smallFont);
}

void GameScreen::Unload(RenderWindow& renderWindow)
{
	_environment.Unload();
	_chestCount.Unload();
	renderWindow.TrapCursor(false);

	_bigFont->Release();
}

void GameScreen::Draw(RenderWindow& renderWindow)
{
	_environment.Draw(renderWindow.GetDevice(), _player.GetCamera());
	_chestCount.Draw(renderWindow);

	if (_environment.NumChests() == 0)
	{
		RECT pos;
		pos.top = 300;
		pos.bottom = pos.top + 72;
		pos.left = 0;
		pos.right = renderWindow.GetSize().x;

		_bigFont->DrawTextA(NULL, "Level Complete!", -1, &pos, DT_CENTER, D3DXCOLOR(1, 1, 1, 1));

		pos.top = pos.bottom + 40;
		pos.bottom = pos.top + 36;
		_smallFont->DrawTextA(NULL, "Press return to continue", -1, &pos, DT_CENTER, D3DXCOLOR(1, 1, 1, 1));
	}
}

void GameScreen::Update(float dt, RenderWindow& renderWindow)
{
	const Input& input = renderWindow.GetInput();
	Vector2f movement;
	Vector2f rotation;


	if (_environment.NumChests() > 0)
	{
		if (input.IsKeyDown(Input::KEY_A)) movement.x = -1;
		if (input.IsKeyDown(Input::KEY_D)) movement.x = 1;
		if (input.IsKeyDown(Input::KEY_W)) movement.y = 1;
		if (input.IsKeyDown(Input::KEY_S)) movement.y = -1;
		if (movement.Length() != 0)
		{
			movement.Normalise();
			movement *= 0.3f;
		}
		rotation.x = input.GetMouseDistance().y * 0.003f;
		rotation.y = input.GetMouseDistance().x* 0.003f;
	}

	_player.Update(movement, rotation, dt, false, input.IsKeyJustPressed(Input::KEY_SPACE));
	_environment.Update(dt);

	for (int i = 0; i <_environment.NumChests(); ++i)
	{
		if(_player.NearChest(_environment.GetChestPosition(i)))
		{
			_environment.RemoveChest(i);
			_chestCount.OnCollected();
		}
	}

	if (_environment.NumChests() == 0 && input.IsKeyJustPressed(Input::KEY_RETURN))
	{
		_parent->NewScreen(new MenuScreen());
	}
}