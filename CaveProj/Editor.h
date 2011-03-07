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
	Editor();

	void Load(RenderWindow& renderWindow);
	void Unload();

	void Draw(RenderWindow& renderWindow);
	void Update(float dt, const Input& input);

	void HandleMessage(MSG msg);

	void SelectLight(int light);
	inline int SelectedLight() { return _selectedLight; }
	void DeselectLight();

private:
	ID3D10ShaderResourceView* _lightIcon;
	Camera _camera;
	BillboardSpriteDrawer _billboardDrawer;
	Environment _environment;
	PositionWidget _positionWidget;
	int _selectedLight;
	EditorUI _editorUI;
};

#endif