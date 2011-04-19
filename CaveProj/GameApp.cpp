#include "GameApp.h"
#include "MenuScreen.h"

GameApp::GameApp() :
	_screenSystem(_renderWindow)
{
	_renderWindow.SetTitle("Cave Explorer");
	_screenSystem.NewScreen(new MenuScreen());
}

void GameApp::LoadGraphics()
{
	_screenSystem.Load();
}

void GameApp::UnloadGraphics()
{
	_screenSystem.Unload();
}

void GameApp::Render()
{
	_screenSystem.Draw();
}

void GameApp::Update(float dt)
{
	_screenSystem.Update(dt);

	if (_screenSystem.CurrentScreen() == NULL)
		_renderWindow.Close();
}
