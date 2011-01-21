#include "Environment.h"
#include "RenderWindow.h"

#include "Camera.h"
#include "ShaderBuilder.h"
#include <cstdlib>

#include <iostream>
#include <vector>

#include "MarchingCubesData.h"
#include "Timer.h"

Environment::Environment(RenderWindow& renderWindow) :
	_renderWindow(renderWindow),
	_genModelEffect(NULL),
	_renderSceneEffect(NULL),
	_genModelTechnique(NULL),
	_renderSceneTechnique(NULL),
	_vertexLayoutGen(NULL),
	_vertexLayoutScene(NULL),
	_bufferPointGrid(NULL),
	_bufferEnvironmentModel(NULL),
	_camera(D3DXVECTOR3(0,0,0), 0, 0),
	_view(NULL),
	_numTriangles(0),
	_resolution(0),
	_texture(0),
	_texture2(0)
{
}

void Environment::GenBlobs()
{
	for (int i = 0; i < 5; ++i)
	{
		_blobs[i].Position = D3DXVECTOR4(rand() % 200 / 100.0f - 1.0f,
			rand() % 200 / 100.0f - 1.0f,
			rand() % 200 / 100.0f - 1.0f,
			0.0f);

		_blobs[i].Radius = rand() % 20 / 24.0f + 0.4f;
	}
	std::cout << "Generated blobs" << std::endl;
}

void Environment::GenModel()
{
	Timer t;
	t.Start();
	ID3D10Device* d3dDevice = _renderWindow.GetDevice();

	ID3D10EffectVariable* blobs = _genModelEffect->GetVariableByName("blobs");

	for (int i = 0; i < 5; ++i)
	{
		ID3D10EffectVariable* blobi = blobs->GetElement(i);

		if (!blobi->IsValid())
			MessageBox(0, "Could not fetch the requested shader variable", "Shader Variable Error", MB_OK);

		// TODO: Error check these variables too (wrap up shader class to do this?)
		blobi->GetMemberByName("Position")->AsVector()->SetFloatVector((float*)&_blobs[i].Position);
		blobi->GetMemberByName("Radius")->AsScalar()->SetFloat(_blobs[i].Radius);
	}

	
	ID3D10Buffer* buffer;

	int m_nBufferSize = 24;

	D3D10_BUFFER_DESC bufferDesc =
	{
		m_nBufferSize,
		D3D10_USAGE_DEFAULT,
		D3D10_BIND_STREAM_OUTPUT | D3D10_BIND_VERTEX_BUFFER,
		0,
		0
	};

	d3dDevice->CreateBuffer(&bufferDesc, NULL, &buffer);

	UINT offset[1] = {0};
	d3dDevice->SOSetTargets( 1, &buffer, offset );

	d3dDevice->IASetInputLayout(_vertexLayoutGen);

	UINT stride = sizeof( D3DVECTOR );
	d3dDevice->IASetVertexBuffers(0, 1, &_bufferPointGrid, &stride, offset);

	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);


	D3D10_QUERY_DESC queryDesc = { D3D10_QUERY_SO_STATISTICS , 0 };

	ID3D10Query* query;
	d3dDevice->CreateQuery(&queryDesc, &query);

	query->Begin();

	D3D10_TECHNIQUE_DESC techDesc;
	_genModelTechnique->GetDesc( &techDesc );
    _genModelTechnique->GetPassByIndex( 0 )->Apply( 0 );
    d3dDevice->Draw( _resolution*_resolution*_resolution, 0 );

	query->End();
	D3D10_QUERY_DATA_SO_STATISTICS soStats;
	while (query->GetData(&soStats, sizeof(soStats), 0) == S_FALSE); // wait for query to be ready (could cause inf loop)

	query->Release();

	if (_bufferEnvironmentModel != NULL)
		_bufferEnvironmentModel->Release();

	bufferDesc.ByteWidth = (UINT)soStats.PrimitivesStorageNeeded * 3 * 28;
	d3dDevice->CreateBuffer(&bufferDesc, NULL, &_bufferEnvironmentModel);
	_numTriangles = (UINT)soStats.PrimitivesStorageNeeded;

	std::cout << "Created Buffer for: " << soStats.PrimitivesStorageNeeded << " triangles" << std::endl;

	d3dDevice->SOSetTargets( 1, &_bufferEnvironmentModel, offset );
	d3dDevice->Draw( _resolution*_resolution*_resolution, 0 );

	d3dDevice->SOSetTargets(0, NULL, NULL);

	t.Stop();
	std::cout << "Model generated in " << t.GetTime() << "s" << std::endl;
}

void Environment::Load()
{
	_resolution = 160;
	float cubeSize = 4.0f / _resolution;
	int limit = _resolution / 2;
	ID3D10Device* d3dDevice = _renderWindow.GetDevice();
	
	// Create Vertex Buffer
	std::vector<D3DVECTOR> inputData;
	inputData.reserve(_resolution*_resolution*_resolution);

	for (int x = -limit; x < limit; ++x)
	{
		for (int y = -limit; y < limit; ++y)
		{
			for (int z = -limit; z < limit; ++z)
			{
				inputData.push_back(D3DXVECTOR3(x*cubeSize, y*cubeSize, z*cubeSize));
			}
		}
	}

	std::cout << "Created point array data" << std::endl;

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = inputData.size()*sizeof(D3DVECTOR);
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &inputData[0];

	if (FAILED(d3dDevice->CreateBuffer( &bd, &InitData, &_bufferPointGrid )))
	{
		MessageBox(0, "Error creating vertex buffer", "Vertex Buffer Error", MB_OK);
	}

	std::cout << "Created point array" << std::endl;

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO) );
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_BC1_UNORM;

	HRESULT hr;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "rock.jpg", &loadInfo, NULL, &_texture, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "rock2.jpg", &loadInfo, NULL, &_texture2, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}
	
	std::cout << "Created textures" << std::endl;

	// Create shader and get render technique
	_renderSceneEffect = ShaderBuilder::RequestEffect("wireframe", "fx_4_0", d3dDevice);

	_renderSceneTechnique = _renderSceneEffect->GetTechniqueByName("Render");

	_genModelEffect = ShaderBuilder::RequestEffect("marchingcubes", "fx_4_0", d3dDevice);

	_genModelTechnique = _genModelEffect->GetTechniqueByName("Render");


	D3D10_PASS_DESC PassDesc;

	D3D10_INPUT_ELEMENT_DESC layoutGen[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElementsGen = sizeof( layoutGen ) / sizeof( layoutGen[0] );

	_genModelTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	d3dDevice->CreateInputLayout( layoutGen, numElementsGen, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &_vertexLayoutGen );

	_renderSceneTechnique->GetPassByIndex( 0 )->GetDesc(&PassDesc);

	D3D10_INPUT_ELEMENT_DESC layoutScene[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElementsScene = sizeof( layoutScene ) / sizeof( layoutScene[0] );
	d3dDevice->CreateInputLayout( layoutScene, numElementsScene, PassDesc.pIAInputSignature,
                                  PassDesc.IAInputSignatureSize, &_vertexLayoutScene );


	// Initialise shader variables

	ID3D10EffectScalarVariable* v = _genModelEffect->GetVariableByName("Edges")->AsScalar();
	v->SetIntArray((int*)MarchingCubesData::Edges, 0, 256);

	v = _genModelEffect->GetVariableByName("TriTable")->AsScalar();
	v->SetIntArray((int*)MarchingCubesData::TriTable, 0, 256*16);

	D3DXMATRIX worldm;
	D3DXMatrixIdentity(&worldm);
	ID3D10EffectMatrixVariable* world = _renderSceneEffect->GetVariableByName("World")->AsMatrix();
	world->SetMatrix((float*)&worldm);

	D3DXMATRIX projm;
	D3DXMatrixPerspectiveFovLH( &projm, ( float )D3DX_PI * 0.25f, _renderWindow.GetSize().x / (float)_renderWindow.GetSize().y, 0.1f, 100.0f );
	ID3D10EffectMatrixVariable* proj = _renderSceneEffect->GetVariableByName("Proj")->AsMatrix();
	proj->SetMatrix((float*)&projm);

	_view = _renderSceneEffect->GetVariableByName("View")->AsMatrix();
	_lightPosition = _renderSceneEffect->GetVariableByName("LightPosition")->AsVector();

	ID3D10EffectScalarVariable* blobCount = _genModelEffect->GetVariableByName("NumBlobs")->AsScalar();
	blobCount->SetInt(5);
	ID3D10EffectScalarVariable* threshold = _genModelEffect->GetVariableByName("Threshold")->AsScalar();
	threshold->SetFloat(3.6f);
	ID3D10EffectScalarVariable* cubeSizeV = _genModelEffect->GetVariableByName("CubeSize")->AsScalar();
	cubeSizeV->SetFloat(cubeSize);

	ID3D10EffectShaderResourceVariable* texturesampler = _renderSceneEffect->GetVariableByName("tex")->AsShaderResource();
	texturesampler->SetResource(_texture);

	texturesampler = _renderSceneEffect->GetVariableByName("tex2")->AsShaderResource();
	texturesampler->SetResource(_texture2);
	NewCave();
}

void Environment::NewCave()
{
	GenBlobs();
	GenModel();
	_camera.Position(D3DXVECTOR3((float*)&_blobs[0].Position));
}

void Environment::Unload()
{
	_genModelTechnique = NULL;
	_renderSceneTechnique = NULL;

	_genModelEffect->Release();
	_genModelEffect = NULL;

	_renderSceneEffect->Release();
	_renderSceneEffect = NULL;

	_vertexLayoutGen->Release();
	_vertexLayoutGen = NULL;

	_vertexLayoutScene->Release();
	_vertexLayoutScene = NULL;

	_texture->Release();
	_texture = NULL;

	_view = NULL;
}

void Environment::Render()
{
	ID3D10Device* d3dDevice = _renderWindow.GetDevice();
	d3dDevice->IASetInputLayout(_vertexLayoutScene);

	UINT stride = sizeof( D3DVECTOR ) * 2;
    UINT offset = 0;
	d3dDevice->IASetVertexBuffers(0, 1, &_bufferEnvironmentModel, &stride, &offset);

	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX viewm = _camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	D3DXVECTOR4 lightpos = D3DXVECTOR4(_camera.Position(), 1.0f);
	_lightPosition->SetFloatVector((float*)lightpos);

	D3D10_TECHNIQUE_DESC techDesc;
	_renderSceneTechnique->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        _renderSceneTechnique->GetPassByIndex( p )->Apply( 0 );
		d3dDevice->Draw( _numTriangles*3, 0 );
    }
}

D3DXVECTOR3 Environment::blobPos(int n)
{
	return D3DXVECTOR3((float*)&_blobs[n].Position);
}

float Environment::sampleField(const D3DXVECTOR3& pos0)
{
	float density = 0;
	
	for (int n = 0; n < 5; ++n)
	{		
		density += _blobs[n].Radius*1/(D3DXVec3Length(&(pos0-blobPos(n)))+0.0001f);
	}

	return density;
}

void Environment::Update(float dt)
{
	const Input& input = _renderWindow.GetInput();

	if (input.IsKeyJustPressed(Input::KEY_SPACE))
	{
		NewCave();
	}

	bool newPos = false;

	D3DXVECTOR3 oldCameraPos = _camera.Position();

	if (input.IsKeyDown(Input::KEY_W))
	{
		_camera.MoveAdvance(dt);
		newPos = true;
	}

	if (input.IsKeyDown(Input::KEY_S))
	{
		_camera.MoveAdvance(-dt);
		newPos = true;
	}

	if (input.IsKeyDown(Input::KEY_A))
	{
		_camera.MoveStrafe(-dt);
		newPos = true;
	}

	if (input.IsKeyDown(Input::KEY_D))
	{
		_camera.MoveStrafe(dt);
		newPos = true;
	}

	if (input.IsButtonDown(Input::BUTTON_LEFT))
	{
		_camera.RotatePitch(input.GetMouseDistance().y*0.006f);
		_camera.RotateYaw(input.GetMouseDistance().x*0.006f);
	}

	if (newPos)
	{
		D3DXVECTOR3 cameraPos = _camera.Position();

		if (sampleField(cameraPos) < 3.6f)
		{
			_camera.Position(oldCameraPos);
		}
	}
}