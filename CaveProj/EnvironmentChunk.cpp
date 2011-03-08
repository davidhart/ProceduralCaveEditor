#include "EnvironmentChunk.h"
#include <iostream>

EnvironmentChunk::EnvironmentChunk(const Vector3f& position, float size, int resolution) :
	_position(position),
	_size(size),
	_resolution(resolution),
	_buffer(NULL),
	_bufferSize(0)
{
}

EnvironmentChunk::~EnvironmentChunk()
{
	Release();
}

float EnvironmentChunk::Importance(const Camera& camera)
{
	Vector3f midPt = _position + Vector3f(_size, _size, _size) / 2.0f;
	Vector3f relativePos = midPt - camera.Position();

	float distance = relativePos.Length();

	float dotLookRelative = camera.Look().Dot(relativePos);

	float perpendiculardistance = (dotLookRelative * camera.Look() - relativePos).Length();

	if (dotLookRelative < -_size) return 0;

	return (1 / perpendiculardistance) * 0.25f + (1 / distance) * 2.0f;
}

void EnvironmentChunk::Generate(ID3D10Device* d3dDevice, ID3D10Effect* generateEffect, ID3D10EffectTechnique* generateTechnique)
{
	generateEffect->GetVariableByName("Size")->AsScalar()->SetInt(_resolution);
	generateEffect->GetVariableByName("CubeSize")->AsScalar()->SetFloat(_size / _resolution);
	D3DXVECTOR4 chunkoffset(_position.x, _position.y, _position.z, 0);
	generateEffect->GetVariableByName("ChunkOffset")->AsVector()->SetFloatVector(&chunkoffset.x);

	generateTechnique->GetPassByIndex(0)->Apply(0);

	ID3D10Buffer* buffer;

	int m_nBufferSize = 24;

	D3D10_BUFFER_DESC bufferDesc =
	{
		m_nBufferSize,
		D3D10_USAGE_DEFAULT,
		D3D10_BIND_STREAM_OUTPUT | D3D10_BIND_VERTEX_BUFFER,
		0,
		0
	};

	d3dDevice->CreateBuffer(&bufferDesc, NULL, &buffer);

	UINT offset[1] = {0};
	d3dDevice->SOSetTargets(1, &buffer, offset);

	D3D10_QUERY_DESC queryDesc = {D3D10_QUERY_SO_STATISTICS, 0};

	ID3D10Query* query;
	d3dDevice->CreateQuery(&queryDesc, &query);

	query->Begin();

	d3dDevice->DrawInstanced(1, _resolution*_resolution*_resolution, 0, 0);

	query->End();
	D3D10_QUERY_DATA_SO_STATISTICS soStats;
	while (query->GetData(&soStats, sizeof(soStats), 0) == S_FALSE); // wait for query to be ready (could cause inf loop)

	query->Release();

	d3dDevice->SOSetTargets(0, NULL, NULL);
	buffer->Release();

	if (_buffer != NULL)
	{
		_buffer->Release();
		_buffer = NULL;
	}
	
	_bufferSize = (UINT)soStats.PrimitivesStorageNeeded * 3 * sizeof(D3DVECTOR)*2;

	if (_bufferSize == 0) // area contained no triangles
		return;

	bufferDesc.ByteWidth = _bufferSize;
	d3dDevice->CreateBuffer(&bufferDesc, NULL, &_buffer);

	//std::cout << "Created Buffer for: " << soStats.PrimitivesStorageNeeded << " triangles" << std::endl;

	d3dDevice->SOSetTargets( 1, &_buffer, offset );

	d3dDevice->DrawInstanced(1, _resolution*_resolution*_resolution, 0, 0);
	
	d3dDevice->SOSetTargets(0, NULL, NULL);
}

void EnvironmentChunk::Draw(ID3D10Device* d3dDevice, ID3D10Effect* renderEffect)
{
	if (_buffer != NULL)
	{
		UINT stride = sizeof(D3DVECTOR) * 2;
		UINT offset = 0;
		d3dDevice->IASetVertexBuffers(0, 1, &_buffer, &stride, &offset);
		d3dDevice->Draw(_bufferSize / stride, 0);
	}
}

void EnvironmentChunk::Release()
{
	if (_buffer != NULL)
	{
		_buffer->Release();
		_buffer = NULL;
		_bufferSize = 0;
	}
}