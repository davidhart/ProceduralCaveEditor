#include "EnvironmentChunk.h"

EnvironmentChunk::EnvironmentChunk(const Vector3f& position, float size, float resolution) :
	_position(position),
	_size(size),
	_resolution(resolution),
	_buffer(NULL),
	_bufferSize(0)
{
}

EnvironmentChunk::~EnvironmentChunk()
{
	if (_buffer != NULL)
		_buffer->Release();
}

float EnvironmentChunk::Importance(const Camera& camera)
{
	return 1.0f / (camera.Position() - (_position + Vector3f(_size, _size, _size) / 2.0f)).Length();
}

void EnvironmentChunk::Generate(ID3D10Device* d3dDevice, ID3D10Effect* generateEffect)
{

}

void EnvironmentChunk::Draw(ID3D10Device* d3dDevice, ID3D10Effect* renderEffect)
{

}