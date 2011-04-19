#include "GameApp.h"

GameApp::GameApp() : 
	_player(_environment)
{
	_renderWindow.SetTitle("Cave Explorer");
	_renderWindow.TrapCursor(true);
}

void GameApp::LoadGraphics()
{
	_player.Load(_renderWindow);
	_environment.Load(_renderWindow.GetDevice(), _player.GetCamera());
	_environment.Open(L"levels/a.cave");
	_chestCount.SetTotal(_environment.NumChests());
	_chestCount.Load(_renderWindow);
}

void GameApp::UnloadGraphics()
{
	_environment.Unload();
	_chestCount.Unload();
}

void GameApp::Render()
{
	_environment.Draw(_renderWindow.GetDevice(), _player.GetCamera());
	_chestCount.Draw(_renderWindow);
}

void GameApp::Update(float dt)
{
	const Input& input = _renderWindow.GetInput();
	Vector2f movement;
	if (input.IsKeyDown(Input::KEY_A)) movement.x = -1;
	if (input.IsKeyDown(Input::KEY_D)) movement.x = 1;
	if (input.IsKeyDown(Input::KEY_W)) movement.y = 1;
	if (input.IsKeyDown(Input::KEY_S)) movement.y = -1;
	if (movement.Length() != 0)
	{
		movement.Normalise();
		movement *= 0.3f;
	}

	for (int i = 0; i <_environment.NumChests(); ++i)
	{
		if(_player.NearChest(_environment.GetChestPosition(i)))
		{
			_environment.RemoveChest(i);
			_chestCount.OnCollected();
		}
	}

	Vector2f rotation (input.GetMouseDistance().y * 0.003f, input.GetMouseDistance().x* 0.003f);

	_player.Update(movement, rotation, dt, false, input.IsKeyJustPressed(Input::KEY_SPACE));
	_environment.Update(dt);
}
