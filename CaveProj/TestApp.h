#pragma once
#ifndef _TESTAPP_H_
#define _TESTAPP_H_

#include "Application.h"
#include "Environment.h"
#include "GwenDX10Renderer.h"

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Controls/Canvas.h"

class TestApp : public Application
{
public:
	TestApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);
	bool HandleMessage(MSG msg);

private:
	Environment _environment;

	Gwen::Skin::TexturedBase _skin;
	Gwen::Renderer::DirectX10* _renderer;
	Gwen::Controls::Canvas* _canvas;
	Gwen::Input::Windows _inputHelper;
	UnitTest* _unit;
};

#endif