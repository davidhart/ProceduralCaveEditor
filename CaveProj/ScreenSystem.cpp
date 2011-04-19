#include "ScreenSystem.h"
#include "RenderWindow.h"

ScreenSystem::ScreenSystem(RenderWindow& renderWindow) :
	_renderWindow(renderWindow),
	_currentScreen(NULL),
	_loaded(false)
{
}

ScreenSystem::~ScreenSystem()
{
	if (_currentScreen != NULL)
		delete _currentScreen;
}

void ScreenSystem::NewScreen(Screen* screen)
{
	if (_currentScreen != NULL)
	{
		if (_loaded)
			_currentScreen->Unload(_renderWindow);

		delete _currentScreen;
	}

	_currentScreen = screen;

	if (_currentScreen != NULL)
	{

		if (_loaded)
			_currentScreen->Load(_renderWindow);

		_currentScreen->_parent = this;
	}
}

void ScreenSystem::Load()
{
	if (!_loaded)
	{
		_loaded = true;

		if (_currentScreen != NULL)
			_currentScreen->Load(_renderWindow);
	}
}

void ScreenSystem::Unload()
{
	if (_loaded)
	{
		_loaded = false;

		if (_currentScreen != NULL)
			_currentScreen->Unload(_renderWindow);
	}
}

void ScreenSystem::Draw()
{
	if (_currentScreen != NULL)
	{
		_currentScreen->Draw(_renderWindow);
	}
}

void ScreenSystem::Update(float dt)
{
	if (_currentScreen != NULL)
	{
		_currentScreen->Update(dt, _renderWindow);
	}
}