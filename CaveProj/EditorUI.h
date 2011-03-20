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
#include <Gwen/Controls/HSVColorPicker.h>
#include <vector>

class RenderWindow;
class Environment;
class Editor;

class EditorUI : public Gwen::Event::Handler
{
public:
	EditorUI(RenderWindow& renderWindow);

	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw();

	void HandleMessage(MSG msg);

	inline void SetEditor(Editor* editor) { _editor = editor; } 
	inline void SetEnvironment(Environment* environment) { _environment = environment; }

	void SelectLight(int light);
	void UpdateLightProperties(int light);

	void SelectShape(int shape);
	void UpdateShapeProperties(int shape);

private:
	void CreateShapePage();
	void CreateNoisePage();
	void CreateLightingPage();
	void CreateWaterPage();
	void CreateObjectsPage();

	void onSaveMenuItem(Gwen::Controls::Base* from);
	void onSaveAsMenuItem(Gwen::Controls::Base* from);
	void onOpenMenuItem(Gwen::Controls::Base* from);
	void onQuitMenuItem(Gwen::Controls::Base* from);
	void onPreviewMenuItem(Gwen::Controls::Base* from);
	void onNewMenuItem(Gwen::Controls::Base* from);

	void PopulateShapeList();
	void onAddShape(Gwen::Controls::Base* from);
	void onRemoveShape(Gwen::Controls::Base* from);
	void onShapeSelected(Gwen::Controls::Base* from);
	void onShapePropertiesChange(Gwen::Controls::Base* from);

	void PopulateLightList();
	void onAddLight(Gwen::Controls::Base* from);
	void onRemoveLight(Gwen::Controls::Base* from);
	void onLightSelected(Gwen::Controls::Base* from);
	void onLightPropertiesChange(Gwen::Controls::Base* from);

	void PopulateOctaveList();
	void onAddOctave(Gwen::Controls::Base* from);
	void onRemoveOctave(Gwen::Controls::Base* from);
	void onOctaveSelected(Gwen::Controls::Base* from);
	void onNoisePropertiesChange(Gwen::Controls::Base* from);
	void SelectOctave(int octave);
	void UpdateNoiseProperties(int octave);

	bool _updatingProperties;
	RenderWindow& _renderWindow;

	std::wstring _currentfilename;

	Editor* _editor;
	Environment* _environment;
	Gwen::Skin::TexturedBase* _skin;
	Gwen::Renderer::DirectX10* _renderer;
	Gwen::Controls::Canvas* _canvas;
	Gwen::Input::Windows _inputHelper;
	Gwen::Controls::DockBase* _dockBase;
	Gwen::Controls::HSVColorPicker* _lightColor;

	Gwen::Controls::ListBox* _shapeList;
	std::vector<Gwen::Controls::Layout::TableRow*> _shapeRows;
	Gwen::Controls::TextBoxNumeric* _shapeXPosition;
	Gwen::Controls::TextBoxNumeric* _shapeYPosition;
	Gwen::Controls::TextBoxNumeric* _shapeZPosition;
	Gwen::Controls::TextBoxNumeric* _shapeXScale;
	Gwen::Controls::TextBoxNumeric* _shapeYScale;
	Gwen::Controls::TextBoxNumeric* _shapeZScale;

	Gwen::Controls::ListBox* _lightsList;
	std::vector<Gwen::Controls::Layout::TableRow*> _lightRows;

	Gwen::Controls::TextBoxNumeric* _lightXPosition;
	Gwen::Controls::TextBoxNumeric* _lightYPosition;
	Gwen::Controls::TextBoxNumeric* _lightZPosition;
	Gwen::Controls::TextBoxNumeric* _lightFalloff;
	Gwen::Controls::TextBoxNumeric* _lightSize;

	Gwen::Controls::ListBox* _octaveList;
	std::vector<Gwen::Controls::Layout::TableRow*> _octaveRows;
	int _selectedOctave;

	Gwen::Controls::TextBoxNumeric* _octaveXScale;
	Gwen::Controls::TextBoxNumeric* _octaveYScale;
	Gwen::Controls::TextBoxNumeric* _octaveZScale;
	Gwen::Controls::TextBoxNumeric* _octaveAmplitude;
};

#endif