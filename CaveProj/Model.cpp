#include "Model.h"
#include "Mesh.h"
#include "Timer.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <limits>

Model::OBJ_INDEX_CACHE::OBJ_INDEX_CACHE() : 
	next(-1),
	packedPos(-1)
{

}

Model::OBJ_FORMAT_INDEX::OBJ_FORMAT_INDEX(int v, int vn, int vt) :
	v(v),
	vn(vn),
	vt(vt),
	hasVn(true),
	hasVt(true)
{

}

Model::OBJ_FORMAT_INDEX::OBJ_FORMAT_INDEX(int v) :
	v(v),
	hasVn(false),
	hasVt(false)
{

}

void Model::OBJ_FORMAT_INDEX::SetVn(int vn)
{
	this->vn = vn;
	hasVn = true;
}

void Model::OBJ_FORMAT_INDEX::SetVt(int vt)
{
	this->vt = vt;
	hasVt = true;
}

Model::Model() :
	_loaded(false),
	_read(false)
{

}

void Model::Read(const std::string& path)
{
	_path = path;

	while (!_meshes.empty())
	{
		delete _meshes.back();
		_meshes.pop_back();
	}

	std::string ext = path.substr(path.find_last_of(".") + 1);

	if (ext == "obj")
	{
		std::ifstream file(path.c_str());

		if (file.good())
		{
			ReadOBJ(file);
			file.close();
			_read = true;
		}
	}
	else
	{
		std::cout << "MODEL (" << path << ") Unknown file extension" << std::endl;

		return;
	}

	if (!_read)
	{
		std::cout << "MODEL (" << path << ") File could not be read" << std::endl;
	}	
}

Model::~Model()
{
	Unload();

	for (unsigned int i = 0; i < _meshes.size(); i++)
	{
		delete _meshes[i];
	}
}

void Model::ReadOBJ(std::ifstream& file)
{
	Timer t;
	t.Start();

	std::vector<Vector3f> vertices;
	std::vector<Vector3f> vertNormals;
	std::vector<Vector2f> vertTextureCoords;
	std::vector<OBJ_FORMAT_INDEX> indices;
	std::string token;
	bool errorEncountered = false;
	std::string material;

	while (!file.eof())
	{
		file >> token;

		if (!file.fail())
		{
			if (token == "v")
			{
				float x, y, z;
				file >> x >> y>> z;

				if (file.fail())
				{
					errorEncountered = true;

					break;
				}

				vertices.push_back(Vector3f(x, y, z));
			}
			else if (token == "vt")
			{
				float s, t;
				file >> s >> t;

				if (file.fail())
				{
					errorEncountered = true;

					break;
				}

				t = 1 - t;

				vertTextureCoords.push_back(Vector2f(s, t));
			}
			else if (token == "vn")
			{
				float x, y, z;
				file >> x >> y>> z;

				if (file.fail())
				{
					errorEncountered = true;

					break;
				}

				vertNormals.push_back(Vector3f(x, y, z));
			}
			else if (token == "f")
			{
				// extract remainder of line to stringstream
				std::stringstream line;
				char c;

				do
				{
					file.get(c);
					line << c;
				}
				while (file.peek() != '\n' && file.good());

				std::string indexstr;
				int firstindex = indices.size();
				int indexcount = 0;

				while (line.good())
				{
					line>>indexstr;

					if (!line.fail())
					{
						OBJ_FORMAT_INDEX newIndex = ParseOBJIndex(indexstr);
						indexcount++;

						// triangle fan -> triangles
						if (indexcount > 3)
						{
							int lastindex = indices.size() - 1;

							// need to add 3 vertices to make triangle, first vertex in poly 
							// + last vertex with new vertex
							indices.push_back(indices[firstindex]);
							indices.push_back(indices[lastindex]);
						}

						indices.push_back(newIndex);
					}
				}
			}
			else if (token == "usemtl")
			{
				if (indices.size() > 0)
				{
					FloatVector packedVertices;
					UintVector packedIndices;
					int vertexFormat;
					ConvertOBJMesh(vertices, vertNormals, vertTextureCoords, indices, packedVertices, packedIndices, vertexFormat);
					_meshes.push_back(new Mesh(packedVertices, packedIndices, vertexFormat));
					indices.clear();
				}

				file >> material;
			}
		}
		else
		{
			file.clear();
		}

		file.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
	}

	if (errorEncountered)
	{
		std::cout << "???" << std::endl;
	}

	if (indices.size() > 0)
	{
		FloatVector packedVertices;
		UintVector packedIndices;
		int vertexFormat;
		ConvertOBJMesh(vertices, vertNormals, vertTextureCoords, indices, packedVertices, packedIndices, vertexFormat);
		_meshes.push_back(new Mesh(packedVertices, packedIndices, vertexFormat));
	}

	t.Stop();

	unsigned int numTri = 0;
	for (unsigned int i = 0; i < _meshes.size(); ++i)
		numTri += _meshes[i]->NumTriangles();

	std::cout << "MODEL (" << _path << ") fetched in " << t.GetTime() << "s [" << _meshes.size() << " surfaces - " << numTri << " triangles]" << std::endl;
}

void Model::ConvertOBJMesh(const Vector3fVector& vertices, const Vector3fVector& normals, const Vector2fVector& texCoords, const ObjIndicesVector& indices,
	FloatVector& packedVertices, UintVector& packedIndices, int& vertexFormatFlags)
{
	bool hasVt = indices[0].hasVt;
	bool hasVn = indices[0].hasVn;

	int stride = 3;

	vertexFormatFlags = Mesh::VERTEX_XYZ;

	if (hasVt)
	{
		stride += 2;
		vertexFormatFlags |= Mesh::VERTEX_TEXTURECOORD;
	}

	if (hasVn)
	{
		stride += 3;
		vertexFormatFlags |= Mesh::VERTEX_NORMAL;
	}

	std::vector<OBJ_INDEX_CACHE> packedIndicesCache(vertices.size());

	for (unsigned int i = 0; i < indices.size(); i++)
	{
		int v = indices[i].v - 1;

		if (packedIndicesCache[v].packedPos == -1)
		{
			packedVertices.push_back(vertices[indices[i].v - 1].x);
			packedVertices.push_back(vertices[indices[i].v - 1].y);
			packedVertices.push_back(vertices[indices[i].v - 1].z);
			packedIndicesCache[v].v = indices[i].v;

			if (hasVt)
			{
				packedVertices.push_back(texCoords[indices[i].vt - 1].x);
				packedVertices.push_back(texCoords[indices[i].vt - 1].y);
				packedIndicesCache[v].vt = indices[i].vt;
			}

			if (hasVn)
			{
				packedVertices.push_back(normals[indices[i].vn - 1].x);
				packedVertices.push_back(normals[indices[i].vn - 1].y);
				packedVertices.push_back(normals[indices[i].vn - 1].z);
				packedIndicesCache[v].vn = indices[i].vn;
			}

			int packedPos = (packedVertices.size() / stride) - 1;
			packedIndices.push_back(packedPos);
			packedIndicesCache[v].packedPos = packedPos;
		}
		else
		{
			bool found = false;
			int index = indices[i].v - 1;

			while (!found)
			{
				if (((!hasVn) || (hasVn && packedIndicesCache[index].vn == indices[i].vn)) && 
					((!hasVt) || (hasVt && packedIndicesCache[index].vt == indices[i].vt)))
				{
					packedIndices.push_back(packedIndicesCache[index].packedPos);
					found = true;

					break;
				}
				else
				{
					if (packedIndicesCache[index].next == -1)
					{
						break;
					}
					else
					{
						index = packedIndicesCache[index].next;
					}
				}
			}

			if (!found)
			{
				packedIndicesCache.push_back(OBJ_INDEX_CACHE());
				packedVertices.push_back(vertices[indices[i].v - 1].x);
				packedVertices.push_back(vertices[indices[i].v - 1].y);
				packedVertices.push_back(vertices[indices[i].v - 1].z);
				packedIndicesCache.back().v = indices[i].v;

				if (hasVt)
				{
					packedVertices.push_back(texCoords[indices[i].vt - 1].x);
					packedVertices.push_back(texCoords[indices[i].vt - 1].y);
					packedIndicesCache.back().vt = indices[i].vt;

				}

				if (hasVn)
				{
					packedVertices.push_back(normals[indices[i].vn - 1].x);
					packedVertices.push_back(normals[indices[i].vn - 1].y);
					packedVertices.push_back(normals[indices[i].vn - 1].z);
					packedIndicesCache.back().vn = indices[i].vn;
				}

				unsigned short packedPos = (packedVertices.size() / stride) - 1;
				packedIndices.push_back((unsigned short)packedPos);
				packedIndicesCache[index].next = packedIndicesCache.size() - 1;
				packedIndicesCache.back().packedPos = packedPos;
			}
		}
	}
}

void Model::Draw(ID3D10Device* d3dDevice)
{
	if (_loaded)
	{
		for (unsigned int i = 0; i < _meshes.size(); i++)
		{
			_meshes[i]->Draw(d3dDevice);
		}
	}
}

void Model::Load(ID3D10Device* d3dDevice)
{
	if (!_loaded && _read)
	{
		Timer t;
		t.Start();

		for (unsigned int i = 0; i < _meshes.size(); i++)
		{
			_meshes[i]->Load(d3dDevice);
		}

		_loaded = true;

		t.Stop();
		
		std::cout << "MODEL (" << _path << ") loaded in " << t.GetTime() << "s" << std::endl;
	}
}

void Model::Unload()
{
	if (_loaded)
	{
		for (unsigned int i = 0; i < _meshes.size(); i++)
		{
			_meshes[i]->Unload();
		}

		_loaded = false;
	}
}

Model::OBJ_FORMAT_INDEX Model::ParseOBJIndex(const std::string& face)
{
	std::stringstream facestrm;
	int v;
	facestrm << face;
	facestrm >> v;

	if (facestrm.eof())
	{
		return OBJ_FORMAT_INDEX(v);
	}

	OBJ_FORMAT_INDEX index(v);

	if (facestrm.get() != '/')
	{
		return index;
	}

	int vt;
	facestrm >> vt;

	if (facestrm.fail())
	{
		facestrm.clear();
	}
	else
	{
		index.SetVt(vt);
	}

	if (facestrm.get() != '/')
	{
		return index;
	}

	int vn;
	facestrm >> vn;

	if (!facestrm.fail())
	{
		index.SetVn(vn);
	}

	return index;
}