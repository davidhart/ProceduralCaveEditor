#include "camera.h"
#include <d3dx9.h>

#include <sstream>

Camera::Camera(const D3DXVECTOR3& position, float pitch, float yaw) : 
	_position (position),
	_pitch(pitch),
	_yaw(yaw)
{
	ViewportSize(Vector2i(1,1));
}

void Camera::ViewportSize(const Vector2i& size)
{
	_viewportSize = size;
	D3DXMatrixPerspectiveFovLH( &_projection, ( float )D3DX_PI * 0.25f, (float)_viewportSize.x / (float)_viewportSize.y, 0.1f, 100.0f );
}

Camera::~Camera()
{

}

D3DXVECTOR3 Camera::Look() const
{
	D3DXVECTOR3 upVec=D3DXVECTOR3(0.0f,1.0f,0.0f);
	D3DXVECTOR3 lookVec=D3DXVECTOR3(0.0f,0.0f,1.0f);
	D3DXVECTOR3 rightVec=D3DXVECTOR3(1.0f,0.0f,0.0f);

	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &upVec, _yaw);
	D3DXVec3TransformCoord(&lookVec, &lookVec, &yawMatrix); 
	D3DXVec3TransformCoord(&rightVec, &rightVec, &yawMatrix); 

	D3DXMATRIX pitchMatrix;
	D3DXMatrixRotationAxis(&pitchMatrix, &rightVec, _pitch);
	D3DXVec3TransformCoord(&lookVec, &lookVec, &pitchMatrix);

	return lookVec;
}

void Camera::MoveAdvance(float distance)
{
	D3DXVECTOR3 lookVec = Look();

	_position += distance * lookVec;
}
void Camera::MoveStrafe(float distance)
{
	D3DXVECTOR3 upVec=D3DXVECTOR3(0.0f,1.0f,0.0f);
	D3DXVECTOR3 rightVec=D3DXVECTOR3(1.0f,0.0f,0.0f);

	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &upVec, _yaw);
	D3DXVec3TransformCoord(&rightVec, &rightVec, &yawMatrix); 

	_position += distance * rightVec;
}
void Camera::RotatePitch(float rotation)
{
	_pitch += rotation;
	if ( _pitch > D3DX_PI/2 )
		_pitch = (float)D3DX_PI/2;

	if ( _pitch < -D3DX_PI/2 )
		_pitch = -(float)D3DX_PI/2;
}
void Camera::RotateYaw(float rotation)
{
	_yaw += rotation;
	while ( _yaw > D3DX_PI*2 )
		_yaw -= (float)D3DX_PI * 2;

	while ( _yaw < 0 )
		_yaw += (float)D3DX_PI * 2;
}

D3DXMATRIX Camera::GetViewMatrix() const
{
	D3DXVECTOR3 upVec= D3DXVECTOR3(0.0f,1.0f,0.0f);
	D3DXVECTOR3 lookVec= D3DXVECTOR3(0.0f,0.0f,1.0f);
	D3DXVECTOR3 rightVec= D3DXVECTOR3(1.0f,0.0f,0.0f);
	
	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &upVec, _yaw);

	D3DXVec3TransformCoord(&lookVec, &lookVec, &yawMatrix); 
	D3DXVec3TransformCoord(&rightVec, &rightVec, &yawMatrix); 

	D3DXMATRIX pitchMatrix;
	D3DXMatrixRotationAxis(&pitchMatrix, &rightVec, _pitch);
	D3DXVec3TransformCoord(&lookVec, &lookVec, &pitchMatrix);
	D3DXVec3TransformCoord(&upVec, &upVec, &pitchMatrix); 
	
	D3DXMATRIX matView;
	D3DXVECTOR3 lookAt = _position + lookVec;
	D3DXMatrixLookAtLH(&matView, &_position, &lookAt, &upVec);

	return matView;
}

const D3DXMATRIX& Camera::GetProjectionMatrix() const
{
	return _projection;
}

std::string Camera::GetDebugString()
{
	std::stringstream ss;
	ss.setf(std::ios_base::floatfield, std::ios_base::fixed);
	ss.precision(2);

	ss << "pos(x: " << _position.x << ", y: " << _position.y << ", z: " << _position.z << ")";
	ss << "  pitch: " << _pitch;
	ss << "  yaw: " << _yaw;
	return ss.str();
}

Ray Camera::UnprojectCoord(const Vector2f& coord) const
{
	D3DXVECTOR3 v;
	v.x = ((2.0f * coord.x / _viewportSize.x) - 1) / _projection._11;
    v.y = -((2.0f * coord.y / _viewportSize.y) - 1) / _projection._22;
    v.z = 1.0f;

	D3DXMATRIX view = GetViewMatrix();
	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &view);

	Vector3f raydirection (v.x * m._11 + v.y * m._21 + v.z * m._31,
						   v.x * m._12 + v.y * m._22 + v.z * m._32,
						   v.x * m._13 + v.y * m._23 + v.z * m._33);

	Vector3f rayorigin (m._41, m._42, m._43);

	return Ray(rayorigin, raydirection);
}