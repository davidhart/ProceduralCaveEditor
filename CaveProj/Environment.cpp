#include "Environment.h"
#include "RenderWindow.h"
#include <ctime>

#include "Camera.h"

Environment::Environment(RenderWindow& renderWindow) :
	_renderWindow(renderWindow),
	_effect(NULL),
	_technique(NULL),
	_vertexLayout(NULL),
	_vertexBuffer(NULL),
	_view(NULL),
	_time(NULL)
{
}

void Environment::Load()
{
	ID3D10Device* d3dDevice = _renderWindow.GetDevice();
	

	// Create Vertex Buffer
	const int size = 40*40*40;
	D3DVECTOR inputData[size];

	int index = 0; 
	for (float x = -2.0f; x < 2.0f; x += 0.1f)
	{
		for (float y = -2.0f; y < 2.0f; y += 0.1f)
		{
			for (float z = -2.0f; z < 2.0f; z += 0.1f)
			{
				inputData[index] = D3DXVECTOR3(x, y, z);
				++index;
			}
		}
	}

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(inputData);
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = inputData;

	if (FAILED(d3dDevice->CreateBuffer( &bd, &InitData, &_vertexBuffer )))
	{
		MessageBox(0, "Error creating vertex buffer", "Vertex Buffer Error", MB_OK);
	}

	// Create shader and get render technique
	ID3D10Blob* error;

	if (FAILED(D3DX10CreateEffectFromFile("marchingcubes.fx",
								0,
								0,
								"fx_4_0",
								D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG,
								0,
								d3dDevice,
								0,
								0,
								&_effect,
								&error,
								0)))
	{
		MessageBox(0, (char*)error->GetBufferPointer(), "Shader Compile Error", MB_OK);
	}

	_technique = _effect->GetTechniqueByName("Render");

	// Create input layout for the first pass of render technique
	D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	D3D10_PASS_DESC PassDesc;

	_technique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	d3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &_vertexLayout );

	// Initialise shader variables
	struct
	{
		D3DXVECTOR4 Position;
		float Radius;
		D3DXVECTOR4 RotationSpeed;
	} blobsData[5];

	blobsData[0].Position	   = D3DXVECTOR4(-0.4f, -0.6f, 0.1f, 0.0f);
	blobsData[0].Radius		   = 0.3f;
	blobsData[0].RotationSpeed = D3DXVECTOR4(-1.0f, -0.4f, 0.0f, 0.0f);

	blobsData[1].Position	   = D3DXVECTOR4(0.4f, 0.3f, 0.0f, 0.0f);
	blobsData[1].Radius		   = 0.2f;
	blobsData[1].RotationSpeed = D3DXVECTOR4(0.2f, 0.0f, 1.0f, 0.0f);

	blobsData[2].Position	   = D3DXVECTOR4(0.4f, 0.3f, -0.2f, 0.0f);
	blobsData[2].Radius		   = 0.1f;
	blobsData[2].RotationSpeed = D3DXVECTOR4(0.0f, -0.7f, 0.3f, 0.0f);

	blobsData[3].Position	   = D3DXVECTOR4(0.1f, 0.4f, 0.0f, 0.0f);
	blobsData[3].Radius		   = 0.05f;
	blobsData[3].RotationSpeed = D3DXVECTOR4(0.4f, 0.0f, 0.2f, 0.0f);

	blobsData[4].Position	   = D3DXVECTOR4(0.0f, 0.6f, 0.1f, 0.0f);
	blobsData[4].Radius		   = 0.05f;
	blobsData[4].RotationSpeed = D3DXVECTOR4(0.1f, 0.0f, 1.0f, 0.0f);

	ID3D10EffectVariable* blobs = _effect->GetVariableByName("blobs");

	for (int i = 0; i < 5; ++i)
	{
		ID3D10EffectVariable* blobi = blobs->GetElement(i);

		if (!blobi->IsValid())
			MessageBox(0, "Could not fetch the requested shader variable", "Shader Variable Error", MB_OK);

		// TODO: Error check these variables too (wrap up shader class to do this?)
		blobi->GetMemberByName("Position")->AsVector()->SetFloatVector((float*)&blobsData[i].Position);
		blobi->GetMemberByName("Radius")->AsScalar()->SetFloat(blobsData[i].Radius);
		blobi->GetMemberByName("RotationSpeed")->AsVector()->SetFloatVector((float*)&blobsData[i].RotationSpeed);
	}

	// TODO: Error check these variables too (wrap up shader class to do this?)
	ID3D10EffectScalarVariable* blobCount = _effect->GetVariableByName("NumBlobs")->AsScalar();
	blobCount->SetInt(5);
	ID3D10EffectScalarVariable* threshold = _effect->GetVariableByName("Threshold")->AsScalar();
	threshold->SetFloat(3.6f);

	_time = _effect->GetVariableByName("Time")->AsScalar();
	_time->SetFloat(0.0f);

	_effect->GetVariableByName("AnimationSpeed")->AsScalar()->SetFloat(0.8f);

	D3DXMATRIX worldm;
	D3DXMatrixIdentity(&worldm);
	ID3D10EffectMatrixVariable* world = _effect->GetVariableByName("World")->AsMatrix();
	world->SetMatrix((float*)&worldm);

	D3DXMATRIX projm;
	D3DXMatrixPerspectiveFovLH( &projm, ( float )D3DX_PI * 0.25f, _renderWindow.Size().x / (float)_renderWindow.Size().y, 0.1f, 100.0f );
	ID3D10EffectMatrixVariable* proj = _effect->GetVariableByName("Proj")->AsMatrix();
	proj->SetMatrix((float*)&projm);

	_view = _effect->GetVariableByName("View")->AsMatrix();

}

void Environment::Unload()
{
	_technique = NULL;
	_effect->Release();
	_effect = NULL;

	_vertexLayout->Release();
	_vertexLayout = NULL;

	_view = NULL;
}

void Environment::Render()
{
	ID3D10Device* d3dDevice = _renderWindow.GetDevice();
	d3dDevice->IASetInputLayout(_vertexLayout);

	UINT stride = sizeof( D3DVECTOR );
    UINT offset = 0;
	d3dDevice->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	Camera camera(D3DXVECTOR3(0, 0, -10), 0, 0);
	D3DXMATRIX viewm = camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	static clock_t st = clock();
	float t = (clock() - st)/(float)CLOCKS_PER_SEC;

	_time->SetFloat(t);

	D3D10_TECHNIQUE_DESC techDesc;
	_technique->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        _technique->GetPassByIndex( p )->Apply( 0 );
        d3dDevice->Draw( 40*40*40, 0 );
    }
}

void Environment::Update(float dt)
{

}