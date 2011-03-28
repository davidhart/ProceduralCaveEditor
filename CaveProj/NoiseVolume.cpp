#include "NoiseVolume.h"
#include "Util.h"
#include <cstdlib>

NoiseVolume::NoiseVolume(const Vector3i& size) :
	_size(size),
	_resourceView(NULL),
	_noiseTexture(NULL)
{
	std::srand(0);
	
	const int numTexels = size.x * size.y * size.z; 
	_noiseTexels.resize(numTexels);

	for (int i = 0; i < numTexels; ++i)
	{
		_noiseTexels[i] = (unsigned char)(std::rand() % 256);
	}
}

void NoiseVolume::Load(ID3D10Device* d3dDevice)
{
	D3D10_TEXTURE3D_DESC texDesc;
	texDesc.Width = _size.x;
	texDesc.Height = _size.y;
	texDesc.Depth = _size.z;
	texDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA data;
	data.pSysMem = &_noiseTexels[0];
	data.SysMemPitch = _size.x;
	data.SysMemSlicePitch = _size.x * _size.y;
	d3dDevice->CreateTexture3D(&texDesc, &data, &_noiseTexture);

	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE3D;
	viewDesc.Format = DXGI_FORMAT_R8_UNORM;
	viewDesc.Texture3D.MipLevels = -1;
	viewDesc.Texture3D.MostDetailedMip = 0;
	d3dDevice->CreateShaderResourceView(_noiseTexture, &viewDesc, &_resourceView);
}

void NoiseVolume::Unload()
{
	_resourceView->Release();
	_resourceView = NULL;

	_noiseTexture->Release();
	_noiseTexture = NULL;
}

float NoiseVolume::ReadTexel(const Vector3i& position)
{
	return _noiseTexels[position.x + position.y * _size.x + position.z * _size.x * _size.y] / 255.0f;
}

float NoiseVolume::Sample(const Vector3f& position)
{
	//Vector3f texelPos ((position.x - 0.5f / _size.x) *  _size.x, (position.y - 0.5f / _size.y) * _size.y, (position.z - 0.5f / _size.z) * _size.z);

	Vector3f texelPos (position.x * _size.x - 0.5f, position.y * _size.y - 0.5f, position.z * _size.z - 0.5f);

	Vector3i texelMin ((int)floor(texelPos.x), (int)floor(texelPos.y), (int)floor(texelPos.z));
	Vector3i texelMax = texelMin + Vector3i(1, 1, 1);
	
	Vector3f lerpAmount(texelPos.x - texelMin.x, texelPos.y - texelMin.y, texelPos.z - texelMin.z);

	texelMin.x %= _size.x; if (texelMin.x < 0) texelMin.x += _size.x;
	texelMin.y %= _size.y; if (texelMin.y < 0) texelMin.y += _size.y;
	texelMin.z %= _size.z; if (texelMin.z < 0) texelMin.z += _size.z;

	texelMax.x %= _size.x; if (texelMax.x < 0) texelMax.x += _size.x;
	texelMax.y %= _size.y; if (texelMax.y < 0) texelMax.y += _size.y;
	texelMax.z %= _size.z; if (texelMax.z < 0) texelMax.z += _size.z;

	// Trilinear interpolation of 8 sample values
	float na = ReadTexel(Vector3i(texelMin.x, texelMin.y, texelMin.z));
	float nb = ReadTexel(Vector3i(texelMax.x, texelMin.y, texelMin.z));
	float nc = ReadTexel(Vector3i(texelMin.x, texelMax.y, texelMin.z));
	float nd = ReadTexel(Vector3i(texelMax.x, texelMax.y, texelMin.z));

	float ne = ReadTexel(Vector3i(texelMin.x, texelMin.y, texelMax.z));
	float nf = ReadTexel(Vector3i(texelMax.x, texelMin.y, texelMax.z));
	float ng = ReadTexel(Vector3i(texelMin.x, texelMax.y, texelMax.z));
	float nh = ReadTexel(Vector3i(texelMax.x, texelMax.y, texelMax.z));

	float la = Util::Lerp(na, nb, lerpAmount.x);
	float lb = Util::Lerp(nc, nd, lerpAmount.x);
	float lc = Util::Lerp(la, lb, lerpAmount.y);

	float ld = Util::Lerp(ne, nf, lerpAmount.x);
	float le = Util::Lerp(ng, nh, lerpAmount.x);
	float lf = Util::Lerp(ld, le, lerpAmount.y);

	return Util::Smoothstep(lc, lf, lerpAmount.z);
}