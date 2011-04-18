#include "RenderWindow.h"
#include "Application.h"

#include <D3DX10.h>
#include <iostream>

const char* RenderWindow::WINDOW_CLASS_NAME = "DXWindow";

RenderWindow::RenderWindow() : 
	_hwnd(0),
	_size(1280, 720),
	_isOpen(false),
	_windowTitle("RenderWindow")
{
	_input.SetWindow(this);
}

bool RenderWindow::IsOpen()
{
	return _isOpen;
}

void RenderWindow::DoEvents(Application& application)
{
	_input.UpdateStep();

	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (!application.HandleMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	_input.PostEvents();
}

bool RenderWindow::Create()
{
	if (!CreateWnd())
		return false;

	if (!CreateDX())
	{
		CleanupWnd();
		return false;
	}

	_isOpen = true;
	ShowWindow(_hwnd, TRUE);

	return true;
}

void RenderWindow::Clear()
{
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	_d3dDevice->ClearRenderTargetView( _renderTargetView, ClearColor );

	_d3dDevice->ClearDepthStencilView( _depthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
}

void RenderWindow::Present()
{
	_d3dSwapChain->Present(0,0);
}

void RenderWindow::Close()
{
	if (_isOpen)
	{
		DestroyWindow(_hwnd);
	}
}

bool RenderWindow::CreateWnd()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = StaticWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = 0;

	if (RegisterClassEx(&wc) == 0)
		return false;

	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD winStyleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = _size.x;
	r.bottom = _size.y;

	AdjustWindowRectEx(&r, winStyle, FALSE, winStyleEx);

	if ((_hwnd = CreateWindowEx(winStyleEx,
							WINDOW_CLASS_NAME,
							_windowTitle.c_str(),
							winStyle,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							r.right - r.left,
							r.bottom - r.top,
							0,
							0,
							GetModuleHandle(0),
							this)) == 0)
	{
		UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(0));
		return false;
	}

	return true;
}

void RenderWindow::CleanupWnd()
{
	DestroyWindow(_hwnd);
	_hwnd = 0;

	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(0));
}

bool RenderWindow::CreateDX()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _size.x;
    sd.BufferDesc.Height = _size.y;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	if (FAILED(D3D10CreateDeviceAndSwapChain(0, 
											D3D10_DRIVER_TYPE_HARDWARE, 
											0,
											D3D10_CREATE_DEVICE_DEBUG, // can specify debug here
											D3D10_SDK_VERSION,
											&sd,
											&_d3dSwapChain,
											&_d3dDevice)))
	{
		return false;
	}


	ID3D10Texture2D* backBuffer;

	if (FAILED(_d3dSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&backBuffer)))
	{
		_d3dSwapChain->Release();
		_d3dSwapChain = 0;
		_d3dDevice->Release();
		_d3dDevice = 0;
		return false;
	}

	HRESULT hr = _d3dDevice->CreateRenderTargetView(backBuffer, 0, &_renderTargetView);

	backBuffer->Release();

	if (FAILED(hr))
	{
		_d3dSwapChain->Release();
		_d3dSwapChain = 0;
		_d3dDevice->Release();
		_d3dDevice = 0;
		return false;
	}


	D3D10_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = _size.x;
    descDepth.Height = _size.y;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = _d3dDevice->CreateTexture2D( &descDepth, NULL, &_depthStencil );

	if (FAILED(hr))
	{
		_d3dSwapChain->Release();
		_d3dSwapChain = 0;
		_d3dDevice->Release();
		_d3dDevice = 0;
		_renderTargetView->Release();
		_renderTargetView = 0;
	}

	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = _d3dDevice->CreateDepthStencilView( _depthStencil, &descDSV, &_depthStencilView );

	if (FAILED(hr))
	{
		MessageBox(0, "Error", "Error", MB_OK);
	}

	_d3dDevice->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);

	D3D10_VIEWPORT vp;
    vp.Width = _size.x;
    vp.Height = _size.y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

	_d3dDevice->RSSetViewports(1, &vp);

	return true;
}

void RenderWindow::CleanupDX()
{
	if (_renderTargetView != 0)
	{
		_renderTargetView->Release();
		_renderTargetView = 0;
	}

	if (_d3dSwapChain != 0)
	{
		_d3dSwapChain->Release();
		_d3dSwapChain = 0;
	}

	if (_d3dDevice != 0)
	{
		_d3dDevice->Release();
		_d3dDevice = 0;
	}
}

void RenderWindow::Cleanup()
{
	CleanupDX();
	CleanupWnd();
}


LRESULT CALLBACK RenderWindow::StaticWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_NCCREATE)
	{
		SetWindowLong(hwnd, GWL_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	else
	{
		RenderWindow* window = (RenderWindow*)GetWindowLongPtr(hwnd, GWL_USERDATA);
		return window->WindowProc(hwnd, msg, wparam, lparam);
	}
}

LRESULT CALLBACK RenderWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		if (_isOpen)
		{
			PostQuitMessage(0);
			Cleanup();
			_isOpen = false;
		}
		break;
	case WM_MOUSEMOVE:
		{
			Vector2f mousePos (LOWORD(lparam), HIWORD(lparam));
			_input.MouseMoveEvent(mousePos);
		}
		break;

	case WM_SETFOCUS:
		_input.SetFocusEvent();
		break;
	case WM_KILLFOCUS:
		_input.LostFocusEvent();
		break;
	case WM_LBUTTONDOWN:
		_input.MouseDownEvent(Input::BUTTON_LEFT);
		break;
	case WM_MBUTTONDOWN:
		_input.MouseDownEvent(Input::BUTTON_MID);
		break;
	case WM_RBUTTONDOWN:
		_input.MouseDownEvent(Input::BUTTON_RIGHT);
		break;
	case WM_LBUTTONUP:
		_input.MouseUpEvent(Input::BUTTON_LEFT);
		break;
	case WM_MBUTTONUP:
		_input.MouseUpEvent(Input::BUTTON_MID);
		break;
	case WM_RBUTTONUP:
		_input.MouseUpEvent(Input::BUTTON_RIGHT);
		break;
	case WM_KEYDOWN:
		if (wparam >= 0 && wparam < 256)
			_input.KeyDownEvent((Input::eKey)wparam);
		break;

	case WM_KEYUP:
		if (wparam >= 0 && wparam < 256)
			_input.KeyUpEvent((Input::eKey)wparam);
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void RenderWindow::SetTitle(const std::string& title)
{
	_windowTitle = title;
	if (_isOpen)
	{
		SetWindowText(_hwnd, title.c_str());
	}
}

void RenderWindow::GetScreenRect(RECT &r) const
{
	GetClientRect(_hwnd, &r);
	ClientToScreen(_hwnd, (LPPOINT)&r.left);
	ClientToScreen(_hwnd, (LPPOINT)&r.right);
}
