#pragma once
#ifndef _BILLBOARDSPRITEDRAWER_H_
#define _BILLBOARDSPRITEDRAWER_H_

#include <d3d10.h>
#include <d3dx10.h>
#include <vector>
#include "Vector.h"

class RenderWindow;
class Camera;

class BillboardSpriteDrawer
{
public:
	BillboardSpriteDrawer();

	void Load(RenderWindow& renderWindow);
	void Unload();

	void Begin(const Camera& camera);
	void Draw(const Vector3f& position, float size, DWORD color, ID3D10ShaderResourceView* texture);
	void End();
	void Flush();

private:
	struct Sprite
	{
		Vector3f _position;
		DWORD _color;
		float _size;
	};

	ID3D10ShaderResourceView* _batchTexture;
	ID3D10Effect* _renderEffect;
	ID3D10EffectTechnique* _renderTechnique;
	ID3D10InputLayout* _vertexLayout;
	ID3D10EffectMatrixVariable* _worldviewprojection;
	ID3D10EffectMatrixVariable* _invview;
	ID3D10EffectShaderResourceVariable* _textureVar;
	ID3D10Device* _device;
	std::vector<Sprite> _sprites;
};

#endif