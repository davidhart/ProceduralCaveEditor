#include "NoiseVolume.h"
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