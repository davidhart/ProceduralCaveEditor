#pragma once
#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <D3D10.h>
#include <D3DX10.h>
#include "Camera.h"

class RenderWindow;

class Environment
{
public:
	Environment(RenderWindow& renderWindow);
	void Load();
	void Unload();
	void Render();
	void Update(float dt);
	void GenBlobs();
	void GenModel();
	void NewCave();

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
	ID3D10EffectVectorVariable* _lightPosition;
	ID3D10ShaderResourceView* _texture;
	UINT _numTriangles;
	int _resolution;

	RenderWindow& _renderWindow;
	Camera _camera;
};

#endif