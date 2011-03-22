#pragma once
#ifndef _NOISEVOLUME_H_
#define _NOISEVOLUME_H_

#include "Vector.h"
#include <D3D10.h>
#include <vector>

class NoiseVolume
{
public:
	NoiseVolume(const Vector3i& size);

	float Sample(const Vector3f& position);
	float ReadTexel(const Vector3i& position);

	void Load(ID3D10Device* d3dDevice);
	void Unload();

	inline ID3D10ShaderResourceView* GetResource() { return _resourceView; }

private:
	Vector3i _size;
	std::vector<unsigned char> _noiseTexels;
	ID3D10ShaderResourceView* _resourceView;
	ID3D10Texture3D* _noiseTexture;
};

#endif