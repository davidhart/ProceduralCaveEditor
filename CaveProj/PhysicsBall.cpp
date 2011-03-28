#include "PhysicsBall.h"
#include "Environment.h"
#include <cstdlib>

PhysicsBall::PhysicsBall(Environment& environment) :
	_environment(environment),
	_radius(0.05f),
	_position(0, 0, 0),
	_velocity(0, 0, 0),
	_inside(false)
{

	std::srand(1231);

	for (int i = 0; i < 16; ++i)
	{
		Vector3f v ((std::rand() % 1000) / 1000.0f,
			(std::rand() % 1000) / 1000.0f,
			(std::rand() % 1000) / 1000.0f);

		v = v.Normalize();

		v.x *= std::rand() % 2 * 2 - 1;
		v.y *= std::rand() % 2 * 2 - 1;
		v.z *= std::rand() % 2 * 2 - 1;

		_samplePositions.push_back(v * _radius);
		_samplePositions.push_back(v * -_radius);
	}

	/*
	_samplePositions.push_back(Vector3f(0, 0, 0));
	_samplePositions.push_back(Vector3f(1, 0, 0)*_radius);
	_samplePositions.push_back(Vector3f(-1, 0, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, -1, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, 1, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, 0, 1)*_radius);
	_samplePositions.push_back(Vector3f(0, 0, -1)*_radius);
	_samplePositions.push_back(Vector3f(0.5, 0, 0)*_radius);
	_samplePositions.push_back(Vector3f(-0.5, 0, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, -0.5, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, 0.5, 0)*_radius);
	_samplePositions.push_back(Vector3f(0, 0, 0.5)*_radius);
	_samplePositions.push_back(Vector3f(0, 0, -0.5)*_radius);*/
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
	const float maxTimeStep = 1 / 250.0f;

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

	Vector3f normal;
	bool collision = false;
	for (unsigned int i = 0; i < _samplePositions.size(); ++i)
	{
		if (_environment.Sample(_position + _samplePositions[i]) < 3.6f)
		{
			normal+= _environment.SampleNormal(_position + _samplePositions[i]);
			collision = true;
		}
	}

	if (collision)
	{
		_position = prevPos;
		normal = normal.Normalize();
		_velocity -= 2 * _velocity.Dot(normal) * normal;
		_velocity *= 0.80f;
	}
}