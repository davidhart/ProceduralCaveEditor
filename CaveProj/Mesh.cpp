#include "Mesh.h"

Mesh::Mesh(const Model::FloatVector& vertices, const Model::UintVector& indices, int vertexFormat) :
	_loaded(false),
	_vertices(vertices),
	_indices(indices),
	_vertexFormat(vertexFormat),
	_vertexBuffer(0),
	_indexBuffer(0)
{

}

Mesh::~Mesh()
{
	Unload();
}

void Mesh::Draw(ID3D10Device* d3dDevice)
{	
	if (_loaded)
	{
		UINT stride = 3 * sizeof(float);

		if (_vertexFormat & Mesh::VERTEX_NORMAL)
			stride += 3 * sizeof(float);

		if (_vertexFormat & Mesh::VERTEX_TEXTURECOORD)
			stride += 2 * sizeof(float);

		UINT offset = 0;

		d3dDevice->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		d3dDevice->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		d3dDevice->DrawIndexed(_indices.size(), 0, 0);
	}
}

void Mesh::Load(ID3D10Device* d3dDevice)
{
	if (!_loaded)
	{
		D3D10_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = _vertices.size() * sizeof(float);
		bufferDesc.Usage = D3D10_USAGE_DEFAULT;
		bufferDesc.MiscFlags = 0;
		bufferDesc.CPUAccessFlags = 0;

		D3D10_SUBRESOURCE_DATA bufferData;
		bufferData.pSysMem = &_vertices[0];

		d3dDevice->CreateBuffer(&bufferDesc, &bufferData, &_vertexBuffer);

		bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = _indices.size() * sizeof(unsigned int);
		bufferData.pSysMem = &_indices[0];

		d3dDevice->CreateBuffer(&bufferDesc, &bufferData, &_indexBuffer);

		_loaded = true;
	}
}

void Mesh::Unload()
{
	if (_loaded)
	{
		_vertexBuffer->Release();
		_vertexBuffer = NULL;
		_indexBuffer->Release();
		_indexBuffer = NULL;

		_loaded = false;
	}
}