#include "BillboardSpriteDrawer.h"
#include "RenderWindow.h"
#include "Camera.h"
#include "ShaderBuilder.h"

BillboardSpriteDrawer::BillboardSpriteDrawer() :
	_renderEffect(NULL),
	_renderTechnique(NULL),
	_vertexLayout(NULL),
	_worldviewprojection(NULL),
	_invview(NULL),
	_device(NULL),
	_batchTexture(NULL),
	_textureVar(NULL)
{
}

void BillboardSpriteDrawer::Load(RenderWindow& renderWindow)
{
	_device = renderWindow.GetDevice();
	_renderEffect = ShaderBuilder::RequestEffect("billboard_sprite", "fx_4_0", _device);
	_renderTechnique = _renderEffect->GetTechniqueByName("Render");

	D3D10_PASS_DESC PassDesc;
	_renderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	D3D10_INPUT_ELEMENT_DESC layoutRender[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	UINT numElements = sizeof(layoutRender) / sizeof(layoutRender[0]);
	_device->CreateInputLayout(layoutRender, numElements, PassDesc.pIAInputSignature,
                                  PassDesc.IAInputSignatureSize, &_vertexLayout);

	_invview = _renderEffect->GetVariableByName("InvView")->AsMatrix();
	_worldviewprojection = _renderEffect->GetVariableByName("WorldViewProjection")->AsMatrix();
	_textureVar = _renderEffect->GetVariableByName("Tex")->AsShaderResource();
}

void BillboardSpriteDrawer::Unload()
{
	_vertexLayout->Release();
	_vertexLayout = NULL;

	_renderEffect->Release();
	_renderEffect = NULL;
	_renderTechnique = NULL;
	
	_worldviewprojection = NULL;
	_invview = NULL;
	_textureVar = NULL;
}

void BillboardSpriteDrawer::Begin(const Camera& camera)
{
	D3DXMATRIX viewm = camera.GetViewMatrix();
	D3DXMATRIX worldviewproj;
	D3DXMatrixMultiply(&worldviewproj, &viewm, &camera.GetProjectionMatrix());
	_worldviewprojection->SetMatrix((float*)&worldviewproj);

	D3DXMATRIX invview;
	D3DXMatrixInverse(&invview, NULL, &viewm);
	_invview->SetMatrix((float*)&invview);

	_renderTechnique->GetPassByIndex(0)->Apply(0);

	_device->IASetInputLayout(_vertexLayout);
	_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void BillboardSpriteDrawer::End()
{
	Flush();
}

void BillboardSpriteDrawer::Draw(const Vector3f& position, float size, DWORD color, ID3D10ShaderResourceView* texture)
{
	if (_batchTexture != texture)
	{
		Flush();
		_batchTexture = texture;
		_textureVar->SetResource(texture);
	}
	Sprite s;
	s._position = position;
	s._size = size;
	s._color = color;
	_sprites.push_back(s);
}

void BillboardSpriteDrawer::Flush()
{
	if (!_sprites.empty())
	{
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = _sprites.size() *sizeof(Sprite);
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &_sprites[0];

		ID3D10Buffer* buffer;
		_device->CreateBuffer(&bd, &InitData, &buffer);
		
		UINT stride = sizeof(Sprite);
		UINT offset = 0;
		_device->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		_device->Draw(_sprites.size(), 0);

		_sprites.clear();
	}
}