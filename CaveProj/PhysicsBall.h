#pragma once
#ifndef _PHYSICSBALL_H_
#define _PHYSICSBALL_H_

#include "Vector.h"
#include "Model.h"
#include <D3D10.h>
#include <vector>
#include "Environment.h"

class RenderWindow;
class Camera;

class PhysicsBall
{
public:
	PhysicsBall(Environment& _environment);
	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw(RenderWindow& renderWindow, const Camera& camera);
	void Update(float dt);
	void Reset();
	inline void SetPosition(const Vector3f& position) { _position = position; _inside = false; }
	inline void SetVelocity(const Vector3f& velocity) { _velocity = velocity; }

private:
	void UpdateStep(float dt);

	Environment& _environment;
	Vector3f _position;
	Vector3f _velocity;
	float _radius;
	bool _inside;
	std::vector<Vector3f> _samplePositions;
	Model _ball;

	ID3D10Effect* _genericEffect;
	ID3D10EffectTechnique* _genericTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10EffectMatrixVariable* _world;
	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectMatrixVariable* _proj;
	ID3D10EffectVectorVariable* _viewDirection;
	Environment::LightParam _lightParams;
};

#endif