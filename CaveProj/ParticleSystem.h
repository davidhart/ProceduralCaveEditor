#pragma once
#ifndef _PARTICLESYSTEM_H_
#define _PARTICLESYSTEM_H_

#include "Environment.h"
#include <D3D10.h>
#include <vector>

class RenderWindow;
class Camera;

class ParticleSystem
{
public:
	ParticleSystem(unsigned int maxParticles, Environment& environment);
	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw(RenderWindow& renderWindow, const Camera& camera);
	void Update(float dt);
	void UpdateStep(float dt);
	void SpawnParticle(unsigned int i);
	void Reset();

private:
	class Particle
	{
	public:
		inline void Update(float dt, Environment& environment)
		{
			_velocity += Vector3f(0, -0.8f, 0) * dt;
			_velocity -= _velocity * 0.4f * dt;

			Vector3f newPos = _position + _velocity * dt;

			if (environment.Sample(newPos) < 3.6f)
			{
				Vector3f normal = environment.SampleNormal(_position);
				normal.Normalize();
				_velocity -= 2 * _velocity.Dot(normal) * normal;
				_velocity *= 0.60f;
			}
			else
			{
				_position = newPos;
			}
		}

		Vector3f _position;
		Vector3f _velocity;
		float _lifespan;
		float _aliveTime;
	};

	unsigned int _maxParticles;
	ID3D10Buffer* _vertexBuffer;
	ID3D10ShaderResourceView* _particleTexture;
	ID3D10Effect* _particleEffect;
	ID3D10EffectTechnique* _particleRenderTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10EffectMatrixVariable* _worldviewprojection;
	ID3D10EffectMatrixVariable* _invview;

	std::vector<Particle> _particles;
	Environment& _environment;

	Environment::LightParam _lightParams;
};

#endif