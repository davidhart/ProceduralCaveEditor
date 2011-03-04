#pragma once
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <D3D10.h>
#include <D3DX10.h>
#include <vector>
#include "Vector.h"

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
	void GenBlobs();
	void GenModel(ID3D10Device* d3dDevice);
	void NewCave(ID3D10Device* d3dDevice);

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

private:
	float sampleField(const D3DXVECTOR3& pos0);
	D3DXVECTOR3 blobPos(int n);

	struct Blob
	{
		D3DXVECTOR4 Position;
		float Radius;
	};

	Blob _blobs[5];

	ID3D10Effect* _genModelEffect;
	ID3D10Effect* _renderSceneEffect;
	ID3D10EffectTechnique* _genModelTechnique;
	ID3D10EffectTechnique* _renderSceneTechnique;
	ID3D10InputLayout* _vertexLayoutGen;
	ID3D10InputLayout* _vertexLayoutScene;
	ID3D10Buffer* _bufferPointGrid;
	ID3D10Buffer* _bufferEnvironmentModel;
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

};

#endif