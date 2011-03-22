#pragma once
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "Vector.h"
#include "EnvironmentChunk.h"
#include "NoiseVolume.h"

#include <D3D10.h>
#include <D3DX10.h>
#include <vector>
#include <string>

class Camera;
class RenderWindow;

class Environment
{
public:
	Environment();
	void Load(ID3D10Device* d3dDevice, Camera& camera);
	void Unload();
	void Draw(ID3D10Device* d3dDevice, Camera& camera);
	void Update(float dt);

	int NumLights() const;
	Vector3f GetLightPosition(int light) const;
	void SetLightPosition(int light, const Vector3f& position);
	DWORD GetLightColor(int light) const;
	void SetLightColor(int light, DWORD color);
	float GetLightFalloff(int light);
	void SetLightFalloff(int light, float falloff);
	float GetLightSize(int light);
	void SetLightSize(int light, float size);
	int AddLight();
	void RemoveLight(int light);

	int NumOctaves() const;
	Vector3f GetOctaveScale(int octave) const;
	void SetOctaveScale(int octave, const Vector3f& scale);
	float GetOctaveAmplitude(int octave) const;
	void SetOctaveAmplitude(int octave, float amplitude);
	int AddOctave();
	void RemoveOctave(int octave);

	int NumShapes() const;
	Vector3f GetShapePosition(int shape) const;
	void SetShapePosition(int shape, const Vector3f& position);
	Vector3f GetShapeScale(int shape) const;
	void SetShapeScale(int shape, const Vector3f& scale);
	int AddShape();
	void RemoveShape(int shape);

	bool Open(const std::wstring& filename);
	bool Save(const std::wstring& filename) const;

	void Rebuild();

	float Sample(const Vector3f& position);
	Vector3f SampleNormal(const Vector3f& position);

private:

	void SortListToGenerate(Camera& camera);
	void GenBlobs();
	void GenModel(ID3D10Device* d3dDevice);
	void NewCave(ID3D10Device* d3dDevice);

	static const int MAX_BLOBS = 5;
	struct Blob
	{
		D3DXVECTOR4 Position;
		D3DXVECTOR4 Scale;
	};

	std::vector<Blob> _shapes;


	static const int MAX_OCTAVES = 5;
	struct Octave
	{
		D3DXVECTOR4 Scale;
		float Amplitude;
	};

	std::vector<Octave> _octaves;

	ID3D10Effect* _genModelEffect;
	ID3D10Effect* _renderSceneEffect;
	ID3D10EffectTechnique* _genModelTechnique;
	ID3D10EffectTechnique* _renderSceneTechnique;
	ID3D10InputLayout* _vertexLayoutGen;
	ID3D10InputLayout* _vertexLayoutScene;
	ID3D10Buffer* _bufferPointGrid;

	std::vector<EnvironmentChunk*> _environmentToGenerate;
	std::vector<EnvironmentChunk*> _environmentToDraw;

	ID3D10EffectMatrixVariable* _view;
	ID3D10EffectVectorVariable* _viewDirection;
	ID3D10ShaderResourceView* _texture;
	ID3D10ShaderResourceView* _textureDisplacement;

	UINT _numTriangles;
	int _resolution;

	void UpdateLights();

	struct Light
	{
		Light();
		Vector3f _position;
		DWORD _color;
		float _size;
		float _falloff;
	};

	std::vector<Light> _lights;
	bool _lightsChanged;

	NoiseVolume _noiseVolume;
};

#endif