#include "TestApp.h"

TestApp::TestApp() : 
	_environment(_renderWindow)
{
}

void TestApp::LoadGraphics()
{
	_renderer = new Gwen::Renderer::DirectX10(_renderWindow.GetDevice());
	_skin.SetRender(_renderer);
	_skin.Init("DefaultSkin.png");
	_canvas = new Gwen::Controls::Canvas(&_skin);
	_canvas->SetSize(1280, 720);

	_unit = new UnitTest(_canvas);
	_unit->SetPos(10, 10);

	_inputHelper.Initialize(_canvas);

	_environment.Load();
}

void TestApp::UnloadGraphics()
{
	_environment.Unload();
}

void TestApp::Render()
{
	_environment.Render();
	_canvas->RenderCanvas();
}

void TestApp::Update(float dt)
{
	_environment.Update(dt);
}

bool TestApp::HandleMessage(MSG msg)
{
	return _inputHelper.ProcessMessage(msg);
}