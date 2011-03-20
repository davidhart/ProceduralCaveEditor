#pragma once
#ifndef _PHYSICSBALL_H_
#define _PHYSICSBALL_H_

#include "DebugDrawer.h"
#include "Vector.h"

class Environment;
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
	inline void SetPosition(const Vector3f& position) { _position = position; _inside = false; }
	inline void SetVelocity(const Vector3f& velocity) { _velocity = velocity; }

private:
	void UpdateStep(float dt);

	DebugDrawer _debugDraw;
	Environment& _environment;
	Vector3f _position;
	Vector3f _velocity;
	float _radius;
	bool _inside;
};

#endif