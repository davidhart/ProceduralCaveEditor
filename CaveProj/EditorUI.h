#pragma once
#ifndef _EDITORUI_H_
#define _EDITORUI_H_

#include "GwenDX10Renderer.h"
#include <Gwen/Gwen.h>
#include <Gwen/Skins/TexturedBase.h>
#include <Gwen/Input/Windows.h>
#include <Gwen/Controls/Canvas.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Events.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/TextBox.h>
#include <vector>

class RenderWindow;
class Environment;
class Editor;

class EditorUI : public Gwen::Event::Handler
{
public:
	EditorUI();

	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw();

	void HandleMessage(MSG msg);

	inline void SetEditor(Editor* editor) { _editor = editor; } 
	inline void SetEnvironment(Environment* environment) { _environment = environment; }

	void SelectLight(int light);
	void UpdateLightProperties(int light);

private:
	void CreateShapePage();
	void CreateNoisePage();
	void CreateLightingPage();
	void CreateWaterPage();
	void CreateObjectsPage();

	void PopulateLightList();

	void onAddLight(Gwen::Controls::Base* from);
	void onRemoveLight(Gwen::Controls::Base* from);
	void onLightSelected(Gwen::Controls::Base* from);

	void onLightPropertiesChange(Gwen::Controls::Base* from);

	Editor* _editor;
	Environment* _environment;
	Gwen::Skin::TexturedBase _skin;
	Gwen::Renderer::DirectX10* _renderer;
	Gwen::Controls::Canvas* _canvas;
	Gwen::Input::Windows _inputHelper;
	Gwen::Controls::DockBase* _dockBase;

	Gwen::Controls::ListBox* _lightsList;
	std::vector<Gwen::Controls::Layout::TableRow*> _lightRows;
	Gwen::Controls::TextBoxNumeric* _lightXPosition;
	Gwen::Controls::TextBoxNumeric* _lightYPosition;
	Gwen::Controls::TextBoxNumeric* _lightZPosition;
	Gwen::Controls::TextBoxNumeric* _lightFalloff;
	Gwen::Controls::TextBoxNumeric* _lightSize;
};

#endif