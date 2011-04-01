#pragma once
#ifndef _DEBUG_DRAWER_H_
#define _DEBUG_DRAWER_H_

#include "Vector.h"
#include <D3D10.h>
#include <D3DX10.h>

class RenderWindow;
class Camera;

class DebugDrawer
{
public:
	void Load(RenderWindow& renderWindow);
	void Unload();

	void DrawLine(const Vector3f& start, const Vector3f& end, RenderWindow& renderWindow, const Camera& camera);

private:
	struct Vertex
	{
		D3DXVECTOR3 pos;
	};

	ID3D10Effect* _renderEffect;
	ID3D10EffectTechnique* _renderTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10EffectMatrixVariable* _world;
	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectMatrixVariable* _proj;
};

#endif