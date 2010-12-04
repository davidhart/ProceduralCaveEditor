#include "Application.h"

#include "Timer.h"

int Application::Run()
{
	if (!_renderWindow.Create())
		return 1;

	LoadGraphics();

	Timer t;
	t.Start();

	while (true)
	{

		_renderWindow.DoEvents();

		if (!_renderWindow.IsOpen())
			break;

		t.Stop();
		t.Start();
		Update(t.GetTime());

		_renderWindow.Clear();

		Render();

		_renderWindow.Present();
	}

	UnloadGraphics();

	return 0;
}