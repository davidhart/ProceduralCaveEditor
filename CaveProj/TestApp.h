#pragma once
#ifndef _TESTAPP_H_
#define _TESTAPP_H_

#include "Application.h"
#include "Environment.h"

class TestApp : public Application
{
public:
	TestApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);

private:
	Environment _environment;
};

#endif