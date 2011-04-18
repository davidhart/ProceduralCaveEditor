#include "EditorApp.h"

EditorApp::EditorApp() : 
	_editor(_renderWindow)
{
	_renderWindow.SetTitle("Cave Editor");
}

void EditorApp::LoadGraphics()
{
	_editor.Load(_renderWindow);
}

void EditorApp::UnloadGraphics()
{
	_editor.Unload();
}

void EditorApp::Render()
{
	_editor.Draw(_renderWindow);
}

void EditorApp::Update(float dt)
{
	_editor.Update(dt, _renderWindow.GetInput());
}

bool EditorApp::HandleMessage(MSG msg)
{
	_editor.HandleMessage(msg);
	return false;
}