#include "Player.h"
#include "Random.h"
#include "RenderWindow.h"
#include "Environment.h"

Player::Player(Environment& environment) : 
	_environment(environment),
	_radius(0.025f),
	_height(0.1f),
	_camera(Vector3f(0, 0, 0), 0, 0),
	_onGround(true)
{
	Random rand;
	rand.Seed(111);

	const Vector3f scale(_radius, _height / 2, _radius);
	const Vector3f altScale(-_radius, -_height / 2, -_radius);

	for (int i = 0; i < 6; ++i)
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
		bool collision = false;

		Vector3f lookVec (0, 0, 1);
		Vector3f upVec(0, 1, 0);
		D3DXMATRIX yawMatrix;

		D3DXMatrixRotationAxis(&yawMatrix, (D3DXVECTOR3*)&upVec, _camera.Yaw());
		D3DXVec3TransformCoord((D3DXVECTOR3*)&lookVec, (D3DXVECTOR3*)&lookVec, &yawMatrix);
		Vector3f strafeVec (lookVec.z, 0, -lookVec.x);

		Vector3f force;
		force += movement.y * lookVec*0.5f;
		force += movement.x * strafeVec*0.5f;

		if (jump && _onGround)
		{
			_velocity += Vector3f(0, 0.5f, 0);
		}

		_velocity += Vector3f(0, -0.8f, 0) * dt;
		_velocity -= _velocity * 0.001f;

		Vector3f normal;
		Vector3f contactNormal;
		
		int it = 0;
		do
		{
			collision = false;
			Vector3f newPosition = position + (_velocity + force) * dt + contactNormal * (it * it * 0.00005f);
			contactNormal = Vector3f(0,0,0);
			for (unsigned int i = 0; i < _samplePositions.size(); ++i)
			{
				if (_environment.Sample(newPosition + _samplePositions[i]) < 3.6f)
				{
					collision = true;
					contactNormal += _environment.SampleNormal(newPosition + _samplePositions[i]);
				}
			}
			if (!collision)
			{
				position = newPosition;
			}
			else
			{
				contactNormal.Normalize();
				_velocity.y = 0;
			}
			++it;

		} while (collision && it < 6);
		//std::cout << it << std::endl;
		
		_onGround = false;

		Vector3f testPosition = position - Vector3f(0, 0.001f, 0);

		for (unsigned int i = 0; i < _samplePositions.size(); ++i)
		{
			if (_environment.Sample(testPosition + _samplePositions[i]) < 3.6f)
			{
				_onGround = true;
				break;
			}
		}
	}

	_camera.Position(position);
}

void Player::Reset()
{
	_camera.Position(Vector3f(0,0,0));
	_camera.PitchYaw(0,0);
	_velocity = Vector3f(0, 0, 0);
}