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

		_renderWindow.DoEvents(*this);

		t.Stop();
		t.Start();
		Update(t.GetTime());
		
		if (!_renderWindow.IsOpen())
			break;

		_renderWindow.Clear();

		Render();

		_renderWindow.Present();
	}

	UnloadGraphics();

	return 0;
}