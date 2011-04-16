#pragma once
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "Vector.h"
#include "EnvironmentChunk.h"
#include "NoiseVolume.h"
#include "GameObject.h"
#include "Model.h"

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
	void New();
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

	static const int MAX_LIGHTS = 8; 

	struct LightParam
	{
		ID3D10EffectVectorVariable* Pos[MAX_LIGHTS];
		ID3D10EffectVectorVariable* Color[MAX_LIGHTS];
		ID3D10EffectScalarVariable* Size[MAX_LIGHTS];
		ID3D10EffectScalarVariable* Falloff[MAX_LIGHTS];
	};

	void LoadLightParameters(const LightParam& lightParam);

private:

	void SortListToGenerate(Camera& camera);
	void GenSurface(ID3D10Device* d3dDevice);

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

	std::vector<EnvironmentChunk*> _environmentToGenerate;
	std::vector<EnvironmentChunk*> _environmentToDraw;

	// Generate Effect Variables
	ID3D10Effect* _genSurfaceEffect;
	ID3D10EffectTechnique* _genSurfaceTechnique;
	ID3D10InputLayout* _vertexLayoutGenSurface;
	ID3D10Buffer* _bufferPoint;

	struct
	{
		struct
		{
			ID3D10EffectVectorVariable* Position;
			ID3D10EffectVectorVariable* Scale;

		} Blobs [MAX_BLOBS];

		struct
		{
			ID3D10EffectVectorVariable* Scale;
			ID3D10EffectScalarVariable* Amplitude;

		} Octaves[MAX_OCTAVES];

		ID3D10EffectScalarVariable* Edges;
		ID3D10EffectScalarVariable* TriTable;
		ID3D10EffectScalarVariable* Threshold;
		ID3D10EffectScalarVariable* NumBlobs;

	} _genSurfaceShaderVars;

	void InitializeSurfaceGenEffect(ID3D10Device* d3dDevice);
	void FetchSurfaceGenShaderVariables();

	// Surface Effect Variables
	ID3D10Effect* _surfaceEffect;
	ID3D10EffectTechnique* _surfaceTechnique;
	ID3D10InputLayout* _vertexLayoutSurface;

	ID3D10ShaderResourceView* _surfaceTexture;
	ID3D10ShaderResourceView* _surfaceDisplacement;

	struct
	{
		ID3D10EffectMatrixVariable* World;
		ID3D10EffectMatrixVariable* Projection;
		ID3D10EffectMatrixVariable* View;
		ID3D10EffectVectorVariable* ViewDirection;
		ID3D10EffectShaderResourceVariable* Texture;
		ID3D10EffectShaderResourceVariable* Displacement;
		LightParam Lights;

	} _surfaceShaderVars;

	void InitializeSurfaceEffect(ID3D10Device* d3dDevice, const Camera& camera);
	void FetchSurfaceShaderVariables();

	// Object Effect Variables
	ID3D10Effect* _objectsEffect;
	ID3D10EffectTechnique* _objectsTechnique;
	ID3D10InputLayout* _vertexLayoutObjects;

	ID3D10ShaderResourceView* _chestTexture;
	ID3D10ShaderResourceView* _treasureTexture;

	Model _chestModel;
	Model _treasureModel;

	struct
	{
		ID3D10EffectMatrixVariable* Projection;
		ID3D10EffectMatrixVariable* View;
		ID3D10EffectMatrixVariable* World;
		ID3D10EffectVectorVariable* ViewDirection;
		ID3D10EffectShaderResourceVariable* Texture;
		LightParam Lights;
	} _objectsShaderVars;
	
	void InitializeObjectsEffect(ID3D10Device* d3dDevice, const Camera& camera);
	void FetchObjectsShaderVariables();

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
	GameObject _testChest;

	NoiseVolume _noiseVolume;
};

#endif