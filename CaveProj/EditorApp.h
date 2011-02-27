#pragma once
#ifndef _EDITORAPP_H_
#define _EDITORAPP_H_

#include "Application.h"
#include "Editor.h"
#include "GwenDX10Renderer.h"

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Controls/Canvas.h"
#include "Gwen/Controls/DockBase.h"

class EditorApp : public Application
{
public:
	EditorApp();
	void LoadGraphics();
	void UnloadGraphics();
	void Render();
	void Update(float dt);
	bool HandleMessage(MSG msg);
	bool IsMouseInUI();

	void CreateShapePage();
	void CreateNoisePage();
	void CreateLightingPage();

private:
	Editor _editor;
	Gwen::Skin::TexturedBase _skin;
	Gwen::Renderer::DirectX10* _renderer;
	Gwen::Controls::Canvas* _canvas;
	Gwen::Input::Windows _inputHelper;
	Gwen::Controls::DockBase* _dockBase;
};

#endif