#pragma once
#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Vector.h"
#include "Camera.h"
#include <vector>

class Environment;
class RenderWindow;

class Player
{
public:
	Player(Environment& environment);
	void Load(RenderWindow& renderWindow);
	void Update(const Vector2f& movement, const Vector2f& rotation,  float dt, bool fly, bool jump);
	void Reset();
	inline Camera& GetCamera() { return _camera; }
	inline const Vector3f& Position() const { return _camera.Position(); }
private:
	std::vector<Vector3f> _samplePositions;
	float _radius;
	float _height;

	Environment& _environment;
	Vector3f _position;
	Vector3f _velocity;
	Camera _camera;
	bool _onGround;
};

#endif