#ifndef _MESH_H_
#define _MESH_H_

#include "Model.h"
#include <D3D10.h>
#include <vector>
#include <string>

class Mesh
{
public:
	enum VertexFormatFlags
	{
		VERTEX_XYZ = 1,
		VERTEX_NORMAL = 2,
		VERTEX_TEXTURECOORD = 4,
	};

	Mesh(const Model::FloatVector& vertices, const Model::UintVector& indices, int vertexFormat);
	virtual ~Mesh();
	void Draw(ID3D10Device* d3dDevice);
	void Load(ID3D10Device* d3dDevice);
	void Unload();
	inline bool IsLoaded() const { return m_loaded; }
	unsigned int NumTriangles() const { return m_indices.size() / 3; }

private:	
	int m_vertexFormat;
	Model::FloatVector m_vertices;
	Model::UintVector m_indices;
	bool m_loaded;

	ID3D10Buffer* _vertexBuffer;
	ID3D10Buffer* _indexBuffer;

};

#endif
