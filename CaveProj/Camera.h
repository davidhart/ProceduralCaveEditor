#pragma once
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <D3DX10.h>
#include <string>

class Camera
{
public:

	Camera(const D3DXVECTOR3& position, float pitch, float yaw);
	~Camera();

	void MoveAdvance(float distance);
	void MoveStrafe(float distance);
	void RotatePitch(float rotation);
	void RotateYaw(float rotation);

	inline void Position(const D3DXVECTOR3& position) { _position = position; }
	inline const D3DXVECTOR3& Position() const { return _position; }

	inline void Pitch(float pitch) { _pitch = pitch; }
	inline float Pitch() { return _pitch; }
	inline void Yaw(float yaw) { _yaw = yaw; }
	inline float Yaw() { return _yaw; }

	D3DXMATRIX GetViewMatrix();
	std::string GetDebugString();

private:

	float _pitch;
	float _yaw;
	D3DXVECTOR3 _position;

};

#endif