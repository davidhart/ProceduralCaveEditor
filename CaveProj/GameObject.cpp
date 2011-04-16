#include "GameObject.h"

GameObject::GameObject() :
	Position(0, 0, 0),
	Rotation(0, 0, 0),
	Scale(1, 1, 1)
{
}

D3DXMATRIX GameObject::GetMatrix()
{
	D3DXMATRIX translate, rotateX, rotateY, rotateZ, scale;
	D3DXMatrixTranslation(&translate, Position.x, Position.y, Position.z);
	D3DXMatrixRotationX(&rotateX, Rotation.x);
	D3DXMatrixRotationY(&rotateY, Rotation.y);
	D3DXMatrixRotationZ(&rotateZ, Rotation.z);
	D3DXMatrixScaling(&scale, Scale.x, Scale.y, Scale.z);

	return (rotateX * rotateY * rotateZ) * scale * translate;
}