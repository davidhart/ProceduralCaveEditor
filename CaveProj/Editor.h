#pragma once
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <D3D10.h>
#include "Camera.h"
#include "BillboardSpriteDrawer.h"
#include "RenderWindow.h"
#include "Environment.h"
#include "PositionWidget.h"

class Environment;

class Editor
{
public:
	Editor();
	void Draw(RenderWindow& renderWindow);

	void Load(RenderWindow& renderWindow);
	void Unload();
	void Update(float dt, const Input& input);

private:
	ID3D10ShaderResourceView* _lightIcon;
	Camera _camera;
	BillboardSpriteDrawer _billboardDrawer;
	Environment _environment;
	PositionWidget _positionWidget;
	int _selectedLight;

};

#endif