#pragma once
#ifndef _POSITION_WIDGET_H_
#define _POSITION_WIDGET_H_

#include "Vector.h"
#include <D3D10.h>
#include <D3DX10.h>

class Ray;
class RenderWindow;
class Camera;
class Input;

class PositionWidget
{
public:
	enum eGrabState
	{
		GRAB_NONE,
		GRAB_X,
		GRAB_Y,
		GRAB_Z,
	};

	PositionWidget(const Vector3f& position);
	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw(const Camera& camera, RenderWindow& renderWindow);
	void Reset();

	inline void SetPosition(const Vector3f& position) { _position = position; }
	inline const Vector3f& GetPosition() const { return _position; }

	eGrabState TestIntersection(const Ray& ray, float& intersection);
	void StartDrag(const Ray& ray, float intersectionPoint, eGrabState grab);
	void HandleDrag(const Camera& camera, const Vector2f& mousePosition);
	void EndDrag();
	inline bool IsInDrag() { return _grabState != GRAB_NONE; }

	inline void SetHover(eGrabState hover) { _hover = hover; }

private:
	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXCOLOR col;
	};

	static const Vertex LineVerts[24];
	static const Vertex PolyVerts[54];

	Vector3f _position;
	Vector3f _grabPoint;
	eGrabState _grabState;
	eGrabState _hover;
	ID3D10Effect* _renderEffect;
	ID3D10EffectTechnique* _renderTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10Buffer* _bufferLines;
	ID3D10Buffer* _bufferPolys;
	ID3D10EffectMatrixVariable* _world;
	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectMatrixVariable* _proj;
	ID3D10EffectVectorVariable* _color;

	UINT _numLines;
	UINT _numPolys;
};

#endif