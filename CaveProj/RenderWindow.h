#pragma once
#ifndef _RENDERWINDOW_H_
#define _RENDERWINDOW_H_

#include <Windows.h>
#include <D3D10.h>
#include "Vector.h"
#include <string>

class RenderWindow
{
public:
	RenderWindow();

	void Close();
	bool IsOpen();
	void DoEvents();

	bool Create();

	void Clear();
	void Present();

	inline ID3D10Device* GetDevice() { return _d3dDevice; }
	inline const Vector2i& Size() const { return _size; }

private:
	bool CreateWnd();
	void CleanupWnd();
	bool CreateDX();
	void CleanupDX();
	void Cleanup();

	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	HWND _hwnd;
	Vector2i _size;
	bool _isOpen;
	std::string _windowTitle;
	ID3D10Device* _d3dDevice;
	IDXGISwapChain* _d3dSwapChain;
	ID3D10RenderTargetView* _renderTargetView;
	ID3D10Texture2D* _depthStencil;
	ID3D10DepthStencilView* _depthStencilView;

	static const char* WINDOW_CLASS_NAME;
};

#endif