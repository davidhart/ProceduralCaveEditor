#ifndef _MODEL_H_
#define _MODEL_H_

#include "Vector.h"
#include <vector>
#include <string>
#include <fstream>
#include <D3D10.h>

class Mesh;

class Model
{
public:
	Model();
	~Model();
	void Read(const std::string& path);
	void Draw(ID3D10Device* d3dDevice);
	void Load(ID3D10Device* d3dDevice);
	void Unload();
	inline bool IsLoaded() const { return m_loaded; }

	typedef std::vector<Vector3f> Vector3fVector;
	typedef std::vector<Vector2f> Vector2fVector;
	typedef std::vector<float> FloatVector;
	typedef std::vector<unsigned int> UintVector;

private:
	class OBJ_INDEX_CACHE
	{
	public:
		int v, vt, vn;
		int next;
		unsigned short packedPos;

		OBJ_INDEX_CACHE();

	};

	class OBJ_FORMAT_INDEX
	{
	public:
		int v;
		int vt;
		int vn;
		bool hasVn;
		bool hasVt;

		OBJ_FORMAT_INDEX(int v);
		OBJ_FORMAT_INDEX(int v, int vt, int vn);
		void SetVt(int vt);
		void SetVn(int vn);

	};

	typedef std::vector<OBJ_FORMAT_INDEX> ObjIndicesVector;

	std::vector<Mesh*> m_meshes;
	std::string m_path;
	bool m_loaded;
	bool m_read;
	unsigned int m_boneBuffer;
	unsigned int m_boneTexture;

	void ReadOBJ(std::ifstream& file);
	static bool ReadToken(std::istream& stream, const std::string& name);
	static bool ReadEnclosedString(std::istream& stream, std::string& string);

	template<typename T> static bool ReadProperty(std::istream& stream, const std::string& name, T& value)
	{
		std::streampos p = stream.tellg();

		if (!ReadToken(stream, name))
		{
			return false;
		}

		stream >> value;

		if (stream.fail())
		{
			stream.seekg(p);
			stream.clear();

			return false;
		}

		return true;
	}

	static OBJ_FORMAT_INDEX ParseOBJIndex(const std::string& face);
	static void ConvertOBJMesh(const Vector3fVector& vertices, const Vector3fVector& normals, const Vector2fVector& texCoords, const ObjIndicesVector& indices, FloatVector& packedVertices, UintVector& packedIndices, int& vertexFormat);

};

#endif
