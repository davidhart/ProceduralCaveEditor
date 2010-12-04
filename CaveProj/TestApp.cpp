#include "TestApp.h"

TestApp::TestApp() : 
	_environment(_renderWindow)
{
}

void TestApp::LoadGraphics()
{
	_environment.Load();
}

void TestApp::UnloadGraphics()
{
	_environment.Unload();
}

void TestApp::Render()
{
	_environment.Render();
}

void TestApp::Update(float dt)
{
	_environment.Update(dt);
}