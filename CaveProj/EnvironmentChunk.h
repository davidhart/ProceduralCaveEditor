#pragma once
#ifndef _ENVIRONMENTCHUNK_H_
#define _ENVIRONMENTCHUNK_H_

#include "AABB.h"
#include "Camera.h"
#include <D3D10.h>

class EnvironmentChunk
{
public:
	EnvironmentChunk(const Vector3f& position, float size, float resolution);
	~EnvironmentChunk();
	float Importance(const Camera& camera);
	void Generate(ID3D10Device* d3dDevice, ID3D10Effect* generateEffect);
	void Draw(ID3D10Device* d3dDevice, ID3D10Effect* renderEffect);
	inline unsigned int MemoryUsage() { return _bufferSize; }

private:
	Vector3f _position;
	float _size;
	float _resolution;
	ID3D10Buffer* _buffer;
	unsigned int _bufferSize;
};

#endif