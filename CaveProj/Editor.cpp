#include "Editor.h"
#include "RenderWindow.h"
#include "Util.h"

#include <iostream>

Editor::Editor(RenderWindow& renderWindow) : 
	_lightIcon(NULL),
	_selectedLight(-1),
	_selectedShape(-1),
	_positionWidget(Vector3f(0, 0, 0)),
	_editorUI(renderWindow),
	_preview(false),
	_ball(_environment),
	_player(_environment),
	_particleSystem(5000, _environment)
{
	_editorUI.SetEnvironment(&_environment);
	_editorUI.SetEditor(this);
}

void Editor::Load(RenderWindow& renderWindow)
{
	_player.Load(renderWindow);
	_environment.Load(renderWindow.GetDevice(), _player.GetCamera());
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
	_environment.Draw(renderWindow.GetDevice(), _player.GetCamera());

	if (_preview)
	{
		_ball.Draw(renderWindow, _player.GetCamera());
		_particleSystem.Draw(renderWindow, _player.GetCamera());
	}
	else
	{
		if (_environment.NumLights() > 0)
		{
			_billboardDrawer.Begin(_player.GetCamera());
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
				_positionWidget.Draw(_player.GetCamera(), renderWindow);
			}
		}

		_editorUI.Draw();
	}
}

void Editor::Update(float dt, const Input& input)
{

	Vector2f movement(0, 0);
	if (input.IsKeyDown(Input::KEY_W)) movement.y = 1;
	if (input.IsKeyDown(Input::KEY_S)) movement.y = -1;
	if (input.IsKeyDown(Input::KEY_A)) movement.x = -1;
	if (input.IsKeyDown(Input::KEY_D)) movement.x = 1;
	if (movement.Length() != 0)
	{
		movement.Normalise();
		movement *= 0.3f;
	}

	Vector2f rotation;
	if (input.IsButtonDown(Input::BUTTON_MID))
	{
		rotation.x = input.GetMouseDistance().y*0.006f;
		rotation.y = input.GetMouseDistance().x*0.006f;
	}

	bool jump = input.IsButtonJustPressed(Input::BUTTON_RIGHT);

	_player.Update(movement, rotation, dt, !_preview, jump);

	if (_preview)
	{

		if (input.IsButtonJustPressed(Input::BUTTON_LEFT))
		{
			_ball.SetPosition(_player.Position());
			_ball.SetVelocity(_player.GetCamera().UnprojectCoord(input.GetCursorPosition())._direction*3.0f);
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
		if (_positionWidget.IsInDrag())
		{
			_positionWidget.HandleDrag(_player.GetCamera(), input.GetCursorPosition());
			_environment.SetLightPosition(_selectedLight, _positionWidget.GetPosition());
			_editorUI.UpdateLightProperties(_selectedLight);
		}

		_positionWidget.SetHover(PositionWidget::GRAB_NONE);

		// Work out what is under the cursor
		Ray r = _player.GetCamera().UnprojectCoord(input.GetCursorPosition());
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
			_positionWidget.SetHover(grab);
			if (input.IsButtonJustPressed(Input::BUTTON_LEFT))
			{
				_positionWidget.StartDrag(r, positionintersect, grab);
			}
		}
		else
		{
			if (input.IsButtonJustPressed(Input::BUTTON_LEFT))
			{
				if (nearestLight >= 0)
				{
					_selectedLight = nearestLight;
					_editorUI.SelectLight(_selectedLight);
				}
			}
		}

		if (input.IsButtonJustReleased(Input::BUTTON_LEFT) && _positionWidget.IsInDrag())
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
	_player.Reset();
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