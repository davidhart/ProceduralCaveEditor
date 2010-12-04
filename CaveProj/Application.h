#pragma once
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "RenderWindow.h"

class Application
{
public:
	int Run();

	virtual void LoadGraphics() = 0;
	virtual void UnloadGraphics() = 0;
	virtual void Render() = 0;
	virtual void Update(float dt) = 0;

protected:
	RenderWindow _renderWindow;
};

#endif