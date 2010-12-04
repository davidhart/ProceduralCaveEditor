#pragma once
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <D3D10.h>
#include <D3DX10.h>

class RenderWindow;

class Environment
{
public:
	Environment(RenderWindow& renderWindow);
	void Load();
	void Unload();
	void Render();
	void Update(float dt);

private:
	ID3D10Effect* _effect;
	ID3D10EffectTechnique* _technique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10Buffer* _vertexBuffer;

	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectScalarVariable* _time;
	RenderWindow& _renderWindow;
};

#endif