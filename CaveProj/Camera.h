#pragma once
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <D3DX10.h>
#include "Vector.h"
#include <string>
#include "Ray.h"

class Camera
{
public:

	Camera(const Vector3f& position, float pitch, float yaw);
	~Camera();

	void MoveAdvance(float distance);
	void MoveStrafe(float distance);
	void RotatePitch(float rotation);

	Vector3f Look() const;

	void RotateYaw(float rotation);

	inline void Position(const Vector3f& position) { _position = position; }
	inline const Vector3f& Position() const { return _position; }

	inline void Pitch(float pitch) { _pitch = pitch; }
	inline float Pitch() { return _pitch; }
	inline void Yaw(float yaw) { _yaw = yaw; }
	inline float Yaw() { return _yaw; }

	D3DXMATRIX GetViewMatrix() const;
	const D3DXMATRIX& GetProjectionMatrix() const;

	std::string GetDebugString();

	Ray UnprojectCoord(const Vector2f& coord) const;

	void ViewportSize(const Vector2i& size);

private:

	float _pitch;
	float _yaw;
	Vector3f _position;
	D3DXMATRIX _projection;
	Vector2i _viewportSize;

};

#endif