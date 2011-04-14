#include "Player.h"
#include "Random.h"
#include "RenderWindow.h"
#include "Environment.h"

Player::Player(Environment& environment) : 
	_environment(environment),
	_radius(0.025f),
	_height(0.1f),
	_camera(Vector3f(0, 0, 0), 0, 0),
	_onGround(false)
{
	Random rand;
	rand.Seed(111);

	const Vector3f scale(_radius, _height / 2, _radius);
	const Vector3f altScale(-_radius, -_height / 2, -_radius);

	for (int i = 0; i < 8; ++i)
	{
		Vector3f v ((rand.Next() % 1000) / 1000.0f,
			(rand.Next() % 1000) / 1000.0f,
			(rand.Next() % 1000) / 1000.0f);

		v.Normalize();

		v.x *= rand.Next() % 2 * 2 - 1;
		v.y *= rand.Next() % 2 * 2 - 1;
		v.z *= rand.Next() % 2 * 2 - 1;

		_samplePositions.push_back(v * scale);
		_samplePositions.push_back(v * altScale);
	}
}

void Player::Load(RenderWindow& renderWindow)
{
	_camera.ViewportSize(renderWindow.GetSize());
}

void Player::Update(const Vector2f& movement, const Vector2f& rotation, float dt, bool fly, bool jump)
{
	_camera.RotatePitch(rotation.x);
	_camera.RotateYaw(rotation.y);
	Vector3f position = _camera.Position();

	if (fly)
	{
		position += _camera.Look() * movement.y * dt;
		position += _camera.Strafe() * movement.x * dt;
		_velocity = Vector3f(0,0,0);
	}
	else
	{
		bool collision;
		int it = 0;
		do
		{
			collision = false;
			Vector3f lookVec (0, 0, 1);
			Vector3f upVec(0, 1, 0);
			D3DXMATRIX yawMatrix;

			D3DXMatrixRotationAxis(&yawMatrix, (D3DXVECTOR3*)&upVec, _camera.Yaw());
			D3DXVec3TransformCoord((D3DXVECTOR3*)&lookVec, (D3DXVECTOR3*)&lookVec, &yawMatrix);
			Vector3f strafeVec (lookVec.z, 0, -lookVec.x);

			Vector3f force;
			force += movement.y * lookVec * dt;
			force += movement.x * strafeVec * dt;
			force += Vector3f(0, -0.2f, 0) * dt;

			if (_onGround)
			{
				_contactNormal.Normalize();
				force -= 2 * force.Dot(_contactNormal) * _contactNormal;

				if (jump)
				{
					force += Vector3f(0, 0.5f, 0);
				}
			}

			_velocity += force;

			_velocity -= _velocity * 0.001f;

			_onGround = false;

			Vector3f normal;
			Vector3f newPosition = position + _velocity * dt;

			bool collision = false;
			for (unsigned int i = 0; i < _samplePositions.size(); ++i)
			{
				if (_environment.Sample(newPosition + _samplePositions[i]) < 3.6f)
				{
					normal += _environment.SampleNormal(newPosition + _samplePositions[i]);
					collision = true;
				}
			}
			if (collision)
			{
				normal.Normalize();
				_contactNormal = normal;

				_velocity -= _velocity.Dot(_contactNormal) * _contactNormal;

				_onGround = true;
				collision = true;
			}
			else
				position = newPosition;
			++it;
		} while (collision && it < 20);
	}


	_camera.Position(position);
}

void Player::Reset()
{
	_camera.Position(Vector3f(0,0,0));
	_camera.PitchYaw(0,0);
	_velocity = Vector3f(0, 0, 0);
}