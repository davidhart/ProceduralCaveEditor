#include "Environment.h"
#include "RenderWindow.h"

#include "Camera.h"
#include "ShaderBuilder.h"
#include "MarchingCubesData.h"
#include "Timer.h"
#include "Util.h"

#include <cstdlib>
#include <iostream>
#include <vector>

Environment::Environment() :
	_genModelEffect(NULL),
	_renderSceneEffect(NULL),
	_genModelTechnique(NULL),
	_renderSceneTechnique(NULL),
	_vertexLayoutGen(NULL),
	_vertexLayoutScene(NULL),
	_bufferPointGrid(NULL),
	_view(NULL),
	_viewDirection(NULL),
	_numTriangles(0),
	_resolution(0),
	_texture(NULL),
	_textureDisplacement(NULL),
	_lightsChanged(false),
	_textureNoise3D(NULL)
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

void Environment::GenModel(ID3D10Device* d3dDevice)
{
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

	d3dDevice->IASetInputLayout(_vertexLayoutGen);

	UINT offset[1] = {0};
	UINT stride = sizeof(D3DVECTOR);
	d3dDevice->IASetVertexBuffers(0, 1, &_bufferPointGrid, &stride, offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	Timer t;

	float Budget = 0.02f;
	// generate models

	for (std::vector<EnvironmentChunk*>::iterator i = _environmentToGenerate.begin(); 
		i != _environmentToGenerate.end();)
	{
		EnvironmentChunk* generated = *i;
		t.Start();
		generated->Generate(d3dDevice, _genModelEffect, _genModelTechnique);
		t.Stop();
		Budget -= t.GetTime();
		
		i = _environmentToGenerate.erase(i);

		_environmentToDraw.push_back(generated);

		if (Budget <= 0)
			break;
	}
}

void Environment::Load(ID3D10Device* d3dDevice, Camera& camera)
{	
	// Create Vertex Buffer
	std::vector<D3DVECTOR> inputData;
	inputData.push_back(D3DXVECTOR3(0, 0, 0));

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

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "rockdisplacement.jpg", &loadInfo, NULL, &_textureDisplacement, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Random3D.dds", &loadInfo, NULL, &_textureNoise3D, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}
	
	std::cout << "Created textures" << std::endl;

	// Create shader and get render technique
	_renderSceneEffect = ShaderBuilder::RequestEffect("cavesurface", "fx_4_0", d3dDevice);

	_renderSceneTechnique = _renderSceneEffect->GetTechniqueByName("Render");

	_genModelEffect = ShaderBuilder::RequestEffect("marchingcubes", "fx_4_0", d3dDevice);

	_genModelTechnique = _genModelEffect->GetTechniqueByName("Render");


	_genModelEffect->GetVariableByName("NoiseTexture")->AsShaderResource()->SetResource(_textureNoise3D);

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

	ID3D10EffectMatrixVariable* proj = _renderSceneEffect->GetVariableByName("Proj")->AsMatrix();
	proj->SetMatrix((float*)&camera.GetProjectionMatrix());

	_view = _renderSceneEffect->GetVariableByName("View")->AsMatrix();
	_viewDirection = _renderSceneEffect->GetVariableByName("ViewDirection")->AsVector();

	ID3D10EffectScalarVariable* blobCount = _genModelEffect->GetVariableByName("NumBlobs")->AsScalar();
	blobCount->SetInt(5);
	ID3D10EffectScalarVariable* threshold = _genModelEffect->GetVariableByName("Threshold")->AsScalar();
	threshold->SetFloat(3.6f);

	ID3D10EffectShaderResourceVariable* texturesampler = _renderSceneEffect->GetVariableByName("tex")->AsShaderResource();
	texturesampler->SetResource(_texture);

	texturesampler = _renderSceneEffect->GetVariableByName("texDisplacement")->AsShaderResource();
	texturesampler->SetResource(_textureDisplacement);

	for (float x = -2; x < 2; x += 1)
		for (float y = -2; y < 2; y += 1)
			for (float z = -2; z < 2; z += 1)
				_environmentToGenerate.push_back(new EnvironmentChunk(Vector3f(x, y, z), 1, 20));

	NewCave(d3dDevice);

	AddLight();
}

void Environment::NewCave(ID3D10Device* d3dDevice)
{
	GenBlobs();
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

	_textureDisplacement->Release();
	_textureDisplacement = NULL;

	_textureNoise3D->Release();
	_textureNoise3D = NULL;

	for(unsigned int i = 0; i < _environmentToDraw.size(); ++i)
		delete _environmentToDraw[i];

	for(unsigned int i = 0; i < _environmentToGenerate.size(); ++i)
		delete _environmentToGenerate[i];

	_view = NULL;
}

void Environment::Draw(ID3D10Device* d3dDevice, Camera& camera)
{
	if (!_environmentToGenerate.empty())
	{
		SortListToGenerate(camera);
		GenModel(d3dDevice);
	}

	if (_lightsChanged)
	{
		UpdateLights();
	}
	d3dDevice->IASetInputLayout(_vertexLayoutScene);

	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX viewm = camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	D3DXVECTOR4 viewDirection = D3DXVECTOR4(*(D3DXVECTOR3*)&camera.Look(), 1.0f);
	_viewDirection->SetFloatVector((float*)viewDirection);
    _renderSceneTechnique->GetPassByIndex(0)->Apply(0);

	for (unsigned int i = 0; i < _environmentToDraw.size(); ++i)
	{
		_environmentToDraw[i]->Draw(d3dDevice, _renderSceneEffect);
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

Environment::Light::Light() : 
	_position(0, 0, 0),
	_color(0xFFFFFFFF),
	_size(0.1f),
	_falloff(5.0f)
{
}

void Environment::SortListToGenerate(Camera& camera)
{
	// To implement
}

void Environment::Rebuild()
{
	for (unsigned int i = 0; i < _environmentToDraw.size(); ++i)
	{
		_environmentToDraw[i]->Release();
	}

	_environmentToGenerate.insert(_environmentToGenerate.end(), _environmentToDraw.begin(), _environmentToDraw.end());
	_environmentToDraw.clear();
}

void Environment::Update(float dt)
{
}

int Environment::NumLights() const
{
	return (int)_lights.size();
}

Vector3f Environment::GetLightPosition(int light) const
{
	return _lights[light]._position;
}

void Environment::SetLightPosition(int light, const Vector3f& position)
{
	_lights[light]._position = position;
	_lightsChanged = true;
}

DWORD Environment::GetLightColor(int light) const
{
	return _lights[light]._color;
}

void Environment::SetLightColor(int light, DWORD color)
{
	_lights[light]._color = color;
	_lightsChanged = true;
}

float Environment::GetLightFalloff(int light)
{
	return _lights[light]._falloff;
}

void Environment::SetLightFalloff(int light, float falloff)
{
	_lights[light]._falloff = falloff;
	_lightsChanged = true;
}

float Environment::GetLightSize(int light)
{
	return _lights[light]._size;
}

void Environment::SetLightSize(int light, float size)
{
	_lights[light]._size = size;
	_lightsChanged = true;
}

int Environment::AddLight()
{
	if (NumLights() < 8)
	{
		_lights.push_back(Light());
		_lightsChanged = true;
		return NumLights() -1;
	}
	return -1;
}

void Environment::RemoveLight(int light)
{
	if (light < 0 || light > NumLights())
		return;

	_lights.erase(_lights.begin()+light);
	_lightsChanged = true;
}

void Environment::UpdateLights()
{
	ID3D10EffectVariable* lights = _renderSceneEffect->GetVariableByName("lights");

	int i;
	for (i = 0; i < NumLights(); ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);

		if (!lighti->IsValid())
		{
			std::cout << "Invalid shader var" << std::endl;
		}

		lighti->GetMemberByName("Position")->AsVector()->SetFloatVector((float*)&_lights[i]._position);

		D3DXCOLOR col(_lights[i]._color);
		lighti->GetMemberByName("Color")->AsVector()->SetFloatVector(&col.r);
	
		lighti->GetMemberByName("Size")->AsScalar()->SetFloat(_lights[i]._size);
	
		lighti->GetMemberByName("Falloff")->AsScalar()->SetFloat(_lights[i]._falloff);
	}

	for (; i < 8; ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);
		D3DXCOLOR col(0,0,0,0);
		lighti->GetMemberByName("Color")->AsVector()->SetFloatVector(&col.r);
	}
}