#include "Editor.h"
#include "RenderWindow.h"
#include "Util.h"

#include <iostream>

Editor::Editor(RenderWindow& renderWindow) : 
	_camera(Vector3f(0,0,0), 0, 0),
	_lightIcon(NULL),
	_selectedLight(-1),
	_selectedShape(-1),
	_positionWidget(Vector3f(0, 0, 0)),
	_editorUI(renderWindow),
	_preview(false),
	_ball(_environment),
	_particleSystem(5000, _environment)
{
	_editorUI.SetEnvironment(&_environment);
	_editorUI.SetEditor(this);
}

void Editor::Load(RenderWindow& renderWindow)
{
	_camera.ViewportSize(renderWindow.GetSize());
	_environment.Load(renderWindow.GetDevice(), _camera);
	_billboardDrawer.Load(renderWindow);

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO) );
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HRESULT hr;

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(renderWindow.GetDevice(), "lightIcon.png", &loadInfo, NULL, &_lightIcon, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}

	_environment.AddLight();
	_environment.SetLightColor(1, COLOR_ARGB(255, 0, 200, 100));
	_environment.SetLightPosition(1, Vector3f(0, -0.3f, 0));
	_positionWidget.Load(renderWindow);

	_editorUI.Load(renderWindow);
	_ball.Load(renderWindow);
	_particleSystem.Load(renderWindow);
}

void Editor::Unload()
{
	_environment.Unload();
	_billboardDrawer.Unload();
	_positionWidget.Unload();
	_lightIcon->Release();
	_lightIcon = NULL;

	_editorUI.Unload();
	_ball.Unload();
	_particleSystem.Unload();
}

void Editor::Draw(RenderWindow& renderWindow)
{
	_environment.Draw(renderWindow.GetDevice(), _camera);

	if (_preview)
	{
		_ball.Draw(renderWindow, _camera);
		_particleSystem.Draw(renderWindow, _camera);
	}
	else
	{
		if (_environment.NumLights() > 0)
		{
			_billboardDrawer.Begin(_camera);
			for (int i = 0; i < _environment.NumLights(); ++i)
			{
				_billboardDrawer.Draw(_environment.GetLightPosition(i),
					0.05f,
					_environment.GetLightColor(i),
					_lightIcon);
			}
			_billboardDrawer.End();

			if (_selectedLight >= 0)
			{
				_positionWidget.SetPosition(_environment.GetLightPosition(_selectedLight));
				_positionWidget.Draw(_camera, renderWindow);
			}
		}

		_editorUI.Draw();
	}
}

void Editor::Update(float dt, const Input& input)
{
	if (input.IsKeyDown(Input::KEY_W))
	{
		_camera.MoveAdvance(dt*0.3f);
	}

	if (input.IsKeyDown(Input::KEY_S))
	{
		_camera.MoveAdvance(-dt*0.3f);
	}

	if (input.IsKeyDown(Input::KEY_A))
	{
		_camera.MoveStrafe(-dt*0.3f);
	}

	if (input.IsKeyDown(Input::KEY_D))
	{
		_camera.MoveStrafe(dt*0.3f);
	}

	if (_preview)
	{
		if (input.IsButtonDown(Input::BUTTON_MID))
		{
			_camera.RotatePitch(input.GetMouseDistance().y*0.006f);
			_camera.RotateYaw(input.GetMouseDistance().x*0.006f);
		}

		if (input.IsButtonJustPressed(Input::BUTTON_LEFT))
		{
			_ball.SetPosition(_camera.Position());
			_ball.SetVelocity(_camera.UnprojectCoord(input.GetCursorPosition())._direction*3.0f);
		}
		
		_ball.Update(dt);
		_particleSystem.Update(dt);

		if (input.IsKeyJustPressed(Input::KEY_ESC))
		{
			Preview(false);
		}
	}
	else
	{
		if (input.IsButtonDown(Input::BUTTON_MID))
		{
			_camera.RotatePitch(input.GetMouseDistance().y*0.006f);
			_camera.RotateYaw(input.GetMouseDistance().x*0.006f);
		}

		// Distance from surface calculation
		//if (_environment.Sample(_camera.Position()) > 3.6f)
		//{
		//	std::cout << (_environment.Sample(_camera.Position())-3.6f) / _environment.SampleNormal(_camera.Position()).Length() << std::endl;
		//}

		if (input.IsButtonJustPressed(Input::BUTTON_LEFT))
		{
			Ray r = _camera.UnprojectCoord(input.GetCursorPosition());
			float nearestPoint = -1;
			int nearestLight = -1;

			for (int i = 0; i < _environment.NumLights(); ++i)
			{
				if (i == _selectedLight)
					continue;

				float t = r.Intersects(_environment.GetLightPosition(i), 0.03f);
				if (t >= 0.0f)
				{
					if (nearestPoint < 0 || t < nearestPoint)
					{
						nearestPoint = t;
						nearestLight = i;
					}
				}
			}

			PositionWidget::eGrabState grab = PositionWidget::GRAB_NONE;
			float positionintersect;

			if (_selectedLight >= 0)
			{
				grab = _positionWidget.TestIntersection(r, positionintersect);
			}

			if (grab != PositionWidget::GRAB_NONE && (positionintersect < nearestPoint || nearestPoint < 0))
			{
				_positionWidget.StartDrag(r, positionintersect, grab);
			}
			else
			{
				if (nearestLight >= 0)
				{
					_selectedLight = nearestLight;
					_editorUI.SelectLight(_selectedLight);
				}
			}
		}

		if (_positionWidget.IsInDrag())
		{
			_positionWidget.HandleDrag(_camera, input.GetCursorPosition());
			_environment.SetLightPosition(_selectedLight, _positionWidget.GetPosition());
			_editorUI.UpdateLightProperties(_selectedLight);
		}

		if (input.IsButtonJustReleased(Input::BUTTON_LEFT))
		{
			_positionWidget.EndDrag();
		}
	}

	if (input.IsKeyJustPressed(Input::KEY_SPACE))
	{
		_environment.Rebuild();
	}
}

void Editor::HandleMessage(MSG msg)
{
	if (!_preview)
	{
		_editorUI.HandleMessage(msg);
	}
}

void Editor::SelectLight(int light)
{
	_selectedLight = light;
	_positionWidget.EndDrag();
	_positionWidget.SetPosition(_environment.GetLightPosition(light));

}

void Editor::DeselectLight()
{
	_positionWidget.EndDrag();
	_selectedLight = -1;
}

void Editor::SelectShape(int shape)
{
	_selectedShape = shape;

	// TODO: 3D controls for shape
}

void Editor::DeselectShape()
{
	_selectedShape = -1;
}

void Editor::ResetCamera()
{
	_camera.Position(Vector3f(0,0,0));
	_camera.PitchYaw(0,0);
}

void Editor::Preview(bool enable)
{
	if (enable && !_preview)
	{
		// TODO: enable stuff
		_particleSystem.Reset();
		_ball.Reset();
	}
	else if (!enable && _preview)
	{
		// disable stuff
	}
	_preview = enable;
}