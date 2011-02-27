#include "DebugDrawer.h"
#include "RenderWindow.h"
#include "Camera.h"
#include "ShaderBuilder.h"

// TODO: Add constructor

void DebugDrawer::Load(RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	_renderEffect = ShaderBuilder::RequestEffect("unlit_vertcol", "fx_4_0", d3dDevice);
	_renderTechnique = _renderEffect->GetTechniqueByName("Render");

	D3D10_PASS_DESC PassDesc;
	_renderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

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
}

void DebugDrawer::Unload()
{
	_view = NULL;
	_proj = NULL;
	_world = NULL;

	_renderTechnique = NULL;
	_renderEffect->Release();
	_renderEffect = NULL;

	_vertexLayout->Release();
	_vertexLayout = NULL;
}

void DebugDrawer::DrawLine(const Vector3f& start, const Vector3f& end, RenderWindow& renderWindow, const Camera& camera)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	Vertex verts[2];
	verts[0].pos = D3DXVECTOR3(start.x, start.y, start.z);
	verts[1].pos = D3DXVECTOR3(end.x, end.y, end.z);
	verts[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	verts[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = 2 *sizeof(Vertex);
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = verts;

	ID3D10Buffer* buffer;
	d3dDevice->CreateBuffer(&bd, &InitData, &buffer);

	D3DXMATRIX worldm;
	D3DXMatrixIdentity(&worldm);
	_world->SetMatrix((float*)&worldm);

	D3DXMATRIX viewm = camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	_proj->SetMatrix((float*)&camera.GetProjectionMatrix());
	
	_renderTechnique->GetPassByIndex(0)->Apply(0);

	UINT stride = sizeof(Vertex);
    UINT offset = 0;

	d3dDevice->IASetInputLayout(_vertexLayout);
	d3dDevice->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	d3dDevice->Draw( 2, 0 );
}