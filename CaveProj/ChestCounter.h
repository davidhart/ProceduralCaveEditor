#pragma once
#ifndef _CHESTCOUNTER_H_
#define _CHESTCOUNTER_H_

#include <d3d10.h>
#include <D3DX10.h>

class RenderWindow;

class ChestCounter
{
public:
	ChestCounter();
	void Load(RenderWindow& renderWindow);
	void Unload();
	void Draw(RenderWindow& renderWindow);

	inline void Reset() { _collected = 0; }
	inline void SetTotal(int total) { _total = total; }
	inline void OnCollected() { ++_collected; }

private:
	unsigned int _total;
	unsigned int _collected;

	ID3DX10Sprite* _sprite;
	ID3DX10Font* _font;
	ID3D10ShaderResourceView* _iconTexture;
};

#endif