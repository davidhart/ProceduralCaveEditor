#include "Application.h"

int Application::Run()
{
	if (!_renderWindow.Create())
		return 1;

	LoadGraphics();

	while (true)
	{
		_renderWindow.DoEvents();

		if (!_renderWindow.IsOpen())
			break;

		Update(0.0f);

		_renderWindow.Clear();

		Render();

		_renderWindow.Present();
	}

	UnloadGraphics();

	return 0;
}