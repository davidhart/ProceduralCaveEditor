#pragma once
#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#include "Vector.h"

#include <D3DX10.h>

class GameObject
{
public:
	GameObject();
	D3DXMATRIX GetMatrix();

	Vector3f Position;
	Vector3f Rotation;
	Vector3f Scale;
};

#endif