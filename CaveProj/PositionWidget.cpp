#include "PositionWidget.h"
#include "RenderWindow.h"
#include "Camera.h"
#include "ShaderBuilder.h"
#include "Input.h"
#include "AABB.h"
#include "Ray.h"
#include "Plane.h"

#include <iostream>

const PositionWidget::Vertex PositionWidget::LineVerts[6] = 
{
	{ D3DXVECTOR3(0, 0, 0), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0, 0), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0, 0, 0), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0, 0.08f, 0), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3(0, 0, 0), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0, 0, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
};

const PositionWidget::Vertex PositionWidget::PolyVerts[54] =
{
	// x
	{ D3DXVECTOR3(0.1f, 0, 0), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, -0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0.1f, 0, 0), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, -0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0.1f, 0, 0), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0.1f, 0, 0), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, -0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, -0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0.08f, -0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, -0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },

	{ D3DXVECTOR3(0.08f, -0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, 0.005f), D3DXCOLOR(1, 0, 0, 1) },
	{ D3DXVECTOR3(0.08f, 0.005f, -0.005f), D3DXCOLOR(1, 0, 0, 1) },

	// y
	{ D3DXVECTOR3(0, 0.1f, 0), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(-0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3( 0, 0.1f, 0), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(-0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3(0, 0.1f, 0), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3(0, 0.1f, 0), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(-0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(-0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3(-0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(-0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },

	{ D3DXVECTOR3(-0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, -0.005f), D3DXCOLOR(0, 1, 0, 1) },
	{ D3DXVECTOR3(0.005f, 0.08f, 0.005f), D3DXCOLOR(0, 1, 0, 1) },

	// z
	{ D3DXVECTOR3(0, 0, 0.1f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(-0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },

	{ D3DXVECTOR3( 0, 0, 0.1f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(-0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },

	{ D3DXVECTOR3(0, 0, 0.1f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },

	{ D3DXVECTOR3(0, 0, 0.1f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(-0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(-0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },

	{ D3DXVECTOR3(-0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(-0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },

	{ D3DXVECTOR3(-0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, 0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
	{ D3DXVECTOR3(0.005f, -0.005f, 0.08f), D3DXCOLOR(0, 0, 1, 1) },
};

PositionWidget::PositionWidget(const Vector3f& position) :
	_position(position),
	_renderTechnique(NULL),
	_vertexLayout(NULL),
	_bufferLines(NULL),
	_bufferPolys(NULL),
	_world(NULL),
	_view(NULL),
	_proj(NULL),
	_renderEffect(NULL),
	_grabState(GRAB_NONE)
{
}

void PositionWidget::Load(RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	_renderEffect = ShaderBuilder::RequestEffect("unlit_vertcol", "fx_4_0", d3dDevice);
	_renderTechnique = _renderEffect->GetTechniqueByName("Render");

	D3D10_PASS_DESC PassDesc;
	_renderTechnique->GetPassByIndex( 0 )->GetDesc(&PassDesc);

	D3D10_INPUT_ELEMENT_DESC layoutRender[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT numElementsRender = sizeof( layoutRender ) / sizeof( layoutRender[0] );
	d3dDevice->CreateInputLayout( layoutRender, numElementsRender, PassDesc.pIAInputSignature,
                                  PassDesc.IAInputSignatureSize, &_vertexLayout );

	_view = _renderEffect->GetVariableByName("View")->AsMatrix();
	_proj = _renderEffect->GetVariableByName("Proj")->AsMatrix();
	_world = _renderEffect->GetVariableByName("World")->AsMatrix();

	_numLines = 3;

	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = _numLines * 2 *sizeof(Vertex);
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = LineVerts;

	if (FAILED(d3dDevice->CreateBuffer( &bd, &InitData, &_bufferLines )))
	{
		MessageBox(0, "Error creating vertex buffer", "Vertex Buffer Error", MB_OK);
	}

	_numPolys = 18;
    bd.ByteWidth = _numPolys * 3 *sizeof(Vertex);
	InitData.pSysMem = PolyVerts;


	if (FAILED(d3dDevice->CreateBuffer( &bd, &InitData, &_bufferPolys )))
	{
		MessageBox(0, "Error creating vertex buffer", "Vertex Buffer Error", MB_OK);
	}
}

void PositionWidget::Unload()
{
	_renderTechnique = NULL;
	_world = NULL;
	_view = NULL;
	_proj = NULL;

	_renderEffect->Release();
	_renderEffect = NULL;

	_vertexLayout->Release();
	_vertexLayout = NULL;

	_bufferLines->Release();
	_bufferLines = NULL;
}

void PositionWidget::Draw(const Camera& camera, RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	D3DXMATRIX worldm;
	D3DXMatrixTranslation(&worldm, _position.x, _position.y, _position.z);
	_world->SetMatrix((float*)&worldm);

	D3DXMATRIX viewm = camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	_proj->SetMatrix((float*)&camera.GetProjectionMatrix());

	_renderTechnique->GetPassByIndex(0)->Apply( 0 );

	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	d3dDevice->IASetVertexBuffers(0, 1, &_bufferPolys, &stride, &offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dDevice->Draw( _numPolys*3, 0 );

	d3dDevice->IASetVertexBuffers(0, 1, &_bufferLines, &stride, &offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	d3dDevice->Draw( _numLines*2, 0 );
}

void PositionWidget::Update(const Camera& camera, const Input& input)
{
	if (_grabState == GRAB_NONE)
	{
		if (input.IsButtonJustPressed(Input::BUTTON_RIGHT))
		{
			Ray r = camera.UnprojectCoord(input.GetCursorPosition());

			Vector3f intersectionPt;

			AABB xArrowBounds(Vector3f(0.08f, -0.005f, -0.005f)+_position, Vector3f(0.1f, 0.005f, 0.005f)+_position);
			AABB yArrowBounds(Vector3f(-0.005f, 0.08f, -0.005f)+_position, Vector3f(0.005f, 0.1f, 0.005f)+_position);
			AABB zArrowBounds(Vector3f(-0.005f, -0.005f, 0.08f)+_position, Vector3f(0.005f, 0.005f, 0.1f)+_position);

			float closestI = -1;
			float t = r.Intersects(xArrowBounds);
			if (t > 0)
			{
				_grabState = GRAB_X;
				closestI = t;
			}
			t = r.Intersects(yArrowBounds);
			if (t > 0 && (t < closestI || closestI < 0))
			{
				_grabState = GRAB_Y;
				closestI = t;
			}
			t = r.Intersects(zArrowBounds);
			if (t > 0 && (t < closestI || closestI < 0))
			{
				_grabState = GRAB_Z;
				closestI = t;
			}

			_grabPoint = (r._origin + r._direction * closestI) - _position;
		}
	}
	else
	{
		Ray r = camera.UnprojectCoord(input.GetCursorPosition());
		Plane p(_position+_grabPoint, Vector3f(0, 0, 0));

		switch (_grabState)
		{
		case GRAB_X:
			p._normal = Vector3f(0, 0, 1);
			break;
		case GRAB_Y:
			p._normal = Vector3f(0, 0, 1);
			break;
		case GRAB_Z:
			p._normal = Vector3f(0, 1, 0);
			break;
		}
		float t = r.Intersects(p, true);
		if (t > 0)
		{
			Vector3f newPos = r._origin + r._direction * t;

			switch (_grabState)
			{
			case GRAB_X:
				_position.x = newPos.x - _grabPoint.x;
				break;
			case GRAB_Y:
				_position.y = newPos.y - _grabPoint.y;
				break;
			case GRAB_Z:
				_position.z = newPos.z - _grabPoint.z;
				break;
			}
		}

		if (input.IsButtonJustReleased(Input::BUTTON_RIGHT))
		{
			_grabState = GRAB_NONE;
		}
	}
}

void PositionWidget::Reset()
{
	_grabState = GRAB_NONE;
}