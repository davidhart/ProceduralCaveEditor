#include "ChestCounter.h"
#include "RenderWindow.h"
#include <string>
#include <sstream>

ChestCounter::ChestCounter() :
	_total (0),
	_collected (0)
{
}

void ChestCounter::Load(RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	D3DX10CreateSprite(d3dDevice, 20, &_sprite);

	D3DX10_FONT_DESCW fd;
	std::wstring facename(L"Tahoma");
	wcscpy_s(fd.FaceName, facename.size()+1, facename.c_str());
	fd.Width = 0;
	fd.MipLevels = 1;
	fd.CharSet = DEFAULT_CHARSET;
	fd.Height = 72;
	fd.OutputPrecision = OUT_DEFAULT_PRECIS;
	fd.Italic = 0;
	fd.Weight = FW_NORMAL;
	fd.Quality = DEFAULT_QUALITY;
	fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	D3DX10CreateFontIndirectW(d3dDevice, &fd, &_font);

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory(&loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO));
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_BC1_UNORM;

	HRESULT hr;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Assets/chesticon.png", &loadInfo, NULL, &_iconTexture, &hr)))
	{
		// TODO: log an error
	}

	D3DXMATRIX projection;
	D3DXMatrixOrthoOffCenterLH(&projection, 0, (float)renderWindow.GetSize().x, (float)renderWindow.GetSize().y, 0, 0, 1);
	_sprite->SetProjectionTransform(&projection);
}

void ChestCounter::Unload()
{
	_sprite->Release();
	_font->Release();
}

void ChestCounter::Draw(RenderWindow& renderWindow)
{
	std::wstringstream ss;
	ss << _collected << "/" << _total;
	
	RECT r;
	r.bottom = renderWindow.GetSize().y;
	r.right = renderWindow.GetSize().x;
	r.left = r.right - 200;
	r.top = r.bottom - 90;
	_font->DrawTextW(NULL, ss.str().c_str(), -1, &r, DT_NOCLIP, D3DXCOLOR(1.0, 1.0f, 1.0f, 1.0f));

	_sprite->Begin(0);

	D3DX10_SPRITE sprite;
	ZeroMemory(&sprite, sizeof(sprite));

	D3DXMatrixTransformation2D(&sprite.matWorld, &D3DXVECTOR2(0,0), 0, &D3DXVECTOR2(64, -64), &D3DXVECTOR2(0, 0), 0, &D3DXVECTOR2((float)renderWindow.GetSize().x - 248, (float)renderWindow.GetSize().y - 48));

	sprite.TexSize = D3DXVECTOR2(1,1);
	sprite.pTexture = _iconTexture;
	sprite.ColorModulate = D3DXCOLOR(1, 1, 1, 1);

	_sprite->DrawSpritesBuffered(&sprite, 1);
	_sprite->End();
}
