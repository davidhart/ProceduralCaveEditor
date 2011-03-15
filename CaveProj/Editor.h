#pragma once
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <D3D10.h>
#include "Camera.h"
#include "BillboardSpriteDrawer.h"
#include "RenderWindow.h"
#include "Environment.h"
#include "EditorUI.h"
#include "PositionWidget.h"

class Environment;

class Editor
{
public:
	Editor(RenderWindow& renderWindow);

	void Load(RenderWindow& renderWindow);
	void Unload();

	void Draw(RenderWindow& renderWindow);
	void Update(float dt, const Input& input);

	void HandleMessage(MSG msg);

	void ResetCamera();

	void SelectLight(int light);
	inline int SelectedLight() { return _selectedLight; }
	void DeselectLight();

	void SelectShape(int shape);
	inline int SelectedShape() { return _selectedShape; }
	void DeselectShape();

private:
	ID3D10ShaderResourceView* _lightIcon;
	Camera _camera;
	BillboardSpriteDrawer _billboardDrawer;
	Environment _environment;
	PositionWidget _positionWidget;
	int _selectedLight;
	int _selectedShape;
	EditorUI _editorUI;
};

#endif