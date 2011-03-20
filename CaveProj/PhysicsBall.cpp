#include "PhysicsBall.h"
#include "Environment.h"

PhysicsBall::PhysicsBall(Environment& environment) :
	_environment(environment),
	_radius(0.05f),
	_position(0, 0, 0),
	_velocity(0, 0, 0),
	_inside(false)
{
}

void PhysicsBall::Load(RenderWindow& renderWindow)
{
	_debugDraw.Load(renderWindow);
}

void PhysicsBall::Unload()
{
	_debugDraw.Unload();
}

void PhysicsBall::Draw(RenderWindow& renderWindow, const Camera& camera)
{
	_debugDraw.DrawLine(_position - Vector3f(_radius, 0, 0), _position + Vector3f(_radius, 0, 0), renderWindow, camera);
	_debugDraw.DrawLine(_position - Vector3f(0, _radius, 0), _position + Vector3f(0, _radius, 0), renderWindow, camera);
	_debugDraw.DrawLine(_position - Vector3f(0, 0, _radius), _position + Vector3f(0, 0, _radius), renderWindow, camera);
}

void PhysicsBall::Update(float dt)
{
	const float maxTimeStep = 1 / 150.0f;

	while (dt > maxTimeStep)
	{
		UpdateStep(maxTimeStep);
		dt -= maxTimeStep;
	}

	UpdateStep(dt);
}

void PhysicsBall::UpdateStep(float dt)
{
	Vector3f prevPos = _position;
	_position += _velocity * dt;
	_velocity += Vector3f(0, -0.005f, 0);
	_velocity -= _velocity * 0.001f;

	Vector3f normal = _environment.SampleNormal(_position);

	if ((_environment.Sample(_position)-3.6f) / normal.Length() < _radius)
	{
		Vector3f nn = normal.Normalize();
		_position = prevPos;
		_velocity -= 2 * _velocity.Dot(nn) * nn;
		_velocity *= 0.90f;
	}
}