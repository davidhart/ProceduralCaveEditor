#pragma once
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <D3D10.h>
#include "BillboardSpriteDrawer.h"
#include "RenderWindow.h"
#include "Environment.h"
#include "EditorUI.h"
#include "PositionWidget.h"
#include "PhysicsBall.h"
#include "Player.h"
#include "ParticleSystem.h"

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

	void SelectObject(int object);
	inline int SelectedObject() { return _selectedObject; }
	void DeselectObject();

	void Preview(bool enable);

private:
	ID3D10ShaderResourceView* _lightIcon;
	BillboardSpriteDrawer _billboardDrawer;
	Environment _environment;
	PositionWidget _positionWidget;
	int _selectedLight;
	int _selectedShape;
	int _selectedObject;
	EditorUI _editorUI;
	bool _preview;
	PhysicsBall _ball;
	ParticleSystem _particleSystem;
	Player _player;
};

#endif