#pragma once
#ifndef _POSITION_WIDGET_H_
#define _POSITION_WIDGET_H_

#include "Vector.h"
#include <D3D10.h>
#include <D3DX10.h>

class RenderWindow;
class Camera;
class Input;

class PositionWidget
{
public:
	PositionWidget(const Vector3f& position);
	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw(const Camera& camera, RenderWindow& renderWindow);
	void Update(const Camera& camera, const Input& input);
	void Reset();

	inline const Vector3f& GetPosition() const { return _position; }

private:
	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXCOLOR col;
	};

	enum eGrabState
	{
		GRAB_NONE,
		GRAB_X,
		GRAB_Y,
		GRAB_Z,
	};

	static const Vertex LineVerts[6];
	static const Vertex PolyVerts[54];

	Vector3f _position;
	Vector3f _grabPoint;
	eGrabState _grabState;
	ID3D10Effect* _renderEffect;
	ID3D10EffectTechnique* _renderTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10Buffer* _bufferLines;
	ID3D10Buffer* _bufferPolys;
	ID3D10EffectMatrixVariable* _world;
	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectMatrixVariable* _proj;

	UINT _numLines;
	UINT _numPolys;
};

#endif