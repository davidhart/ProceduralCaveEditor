#include "Environment.h"
#include "RenderWindow.h"

#include "Camera.h"
#include "ShaderBuilder.h"
#include "MarchingCubesData.h"
#include "Timer.h"
#include "Util.h"
#include "Random.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

Environment::Environment() :
	_lightsChanged(false),
	_noiseVolume(Vector3i(256, 256, 256)),
	_elapsed(0)
{
}

void Environment::Load(ID3D10Device* d3dDevice, Camera& camera)
{	
	_noiseVolume.Load(d3dDevice);

	InitializeSurfaceGenEffect(d3dDevice);
	InitializeSurfaceEffect(d3dDevice, camera);
	InitializeObjectsEffect(d3dDevice, camera);

	float size = 1.5f;

	for (float x = -2; x < 2; x += 1)
		for (float y = -2; y < 2; y += 1)
			for (float z = -2; z < 2; z += 1)
				_environmentToGenerate.push_back(new EnvironmentChunk(Vector3f(x*size, y*size, z*size), size, 32));

	New();
}

// Initialise surface generation effect, effect input layout, shader variables and resources required 
// by the effect
void Environment::InitializeSurfaceGenEffect(ID3D10Device* d3dDevice)
{
	_genSurfaceEffect = ShaderBuilder::RequestEffect("Assets/marchingcubes", "fx_4_0", d3dDevice);
	_genSurfaceTechnique = _genSurfaceEffect->GetTechniqueByName("Render");
	_genSurfaceEffect->GetVariableByName("NoiseTexture")->AsShaderResource()->SetResource(_noiseVolume.GetResource());

	D3D10_PASS_DESC PassDesc;
	D3D10_INPUT_ELEMENT_DESC layoutGen[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
    };
    UINT numElementsGen = sizeof(layoutGen) / sizeof(layoutGen[0]);

	_genSurfaceTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	d3dDevice->CreateInputLayout(layoutGen, numElementsGen, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &_vertexLayoutGenSurface);

	// This buffer is created to hack around the fact that I have been unable to find a way
	// to call the surface gen effect without a buffer being bound (as it does not require any 
	// input other than SV_InstanceID
	std::vector<D3DVECTOR> inputData;
	inputData.push_back(D3DXVECTOR3(0, 0, 0));

    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = inputData.size()*sizeof(D3DVECTOR);
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &inputData[0];

	d3dDevice->CreateBuffer(&bd, &InitData, &_bufferPoint);

	FetchSurfaceGenShaderVariables();
	
	_genSurfaceShaderVars.TriTable->SetIntArray((int*)MarchingCubesData::TriTable, 0, 256*16);
	_genSurfaceShaderVars.Threshold->SetFloat(3.6f);
}

void Environment::FetchSurfaceGenShaderVariables()
{
	ID3D10EffectVariable* blobs = _genSurfaceEffect->GetVariableByName("blobs");
	for (int i = 0; i < MAX_BLOBS; ++i)
	{
		ID3D10EffectVariable* blobi = blobs->GetElement(i);
		_genSurfaceShaderVars.Blobs[i].Position = blobi->GetMemberByName("Position")->AsVector();
		_genSurfaceShaderVars.Blobs[i].Scale = blobi->GetMemberByName("Scale")->AsVector();
	}

	_genSurfaceShaderVars.NumBlobs = _genSurfaceEffect->GetVariableByName("NumBlobs")->AsScalar();

	ID3D10EffectVariable* octaves = _genSurfaceEffect->GetVariableByName("octaves");
	for (int i = 0; i < MAX_OCTAVES; ++i)
	{
		ID3D10EffectVariable* octavei = octaves->GetElement(i);
		_genSurfaceShaderVars.Octaves[i].Scale = octavei->GetMemberByName("Scale")->AsVector();
		_genSurfaceShaderVars.Octaves[i].Amplitude = octavei->GetMemberByName("Amplitude")->AsScalar();
	}

	_genSurfaceShaderVars.TriTable = _genSurfaceEffect->GetVariableByName("TriTable")->AsScalar();
	_genSurfaceShaderVars.Threshold = _genSurfaceEffect->GetVariableByName("Threshold")->AsScalar();
}

// Initialise surface render effect, effect input layout, shader variables and resources required 
// by the effect
void Environment::InitializeSurfaceEffect(ID3D10Device* d3dDevice, const Camera& camera)
{
	_surfaceEffect = ShaderBuilder::RequestEffect("Assets/cavesurface", "fx_4_0", d3dDevice);
	_surfaceTechnique = _surfaceEffect->GetTechniqueByName("Render");
	
	D3D10_PASS_DESC PassDesc;
	_surfaceTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	D3D10_INPUT_ELEMENT_DESC layoutSurface[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
    };

    UINT numElementsScene = sizeof(layoutSurface) / sizeof(layoutSurface[0]);
	d3dDevice->CreateInputLayout(layoutSurface, numElementsScene, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &_vertexLayoutSurface);

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory(&loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO));
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_BC1_UNORM;

	HRESULT hr;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Assets/rock.jpg", &loadInfo, NULL, &_surfaceTexture, &hr)))
	{
		// TODO: log an error
	}

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Assets/rockdisplacement.jpg", &loadInfo, NULL, &_surfaceDisplacement, &hr)))
	{
		// TODO: log an error
	}

	FetchSurfaceShaderVariables();

	_surfaceShaderVars.Projection->SetMatrix((float*)&camera.GetProjectionMatrix());
	_surfaceShaderVars.Texture->SetResource(_surfaceTexture);
	_surfaceShaderVars.Displacement->SetResource(_surfaceDisplacement);
}

void Environment::FetchSurfaceShaderVariables()
{
	_surfaceShaderVars.View = _surfaceEffect->GetVariableByName("View")->AsMatrix();
	_surfaceShaderVars.ViewDirection = _surfaceEffect->GetVariableByName("ViewDirection")->AsVector();
	_surfaceShaderVars.Projection = _surfaceEffect->GetVariableByName("Proj")->AsMatrix();
	_surfaceShaderVars.Texture = _surfaceEffect->GetVariableByName("Texture")->AsShaderResource();
	_surfaceShaderVars.Displacement = _surfaceEffect->GetVariableByName("Displacement")->AsShaderResource();

	ID3D10EffectVariable* lights = _surfaceEffect->GetVariableByName("Lights");
	int i;
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);
		_surfaceShaderVars.Lights.Pos[i] = lighti->GetMemberByName("Position")->AsVector();
		_surfaceShaderVars.Lights.Color[i] = lighti->GetMemberByName("Color")->AsVector();
		_surfaceShaderVars.Lights.Size[i] = lighti->GetMemberByName("Size")->AsScalar();
		_surfaceShaderVars.Lights.Falloff[i] = lighti->GetMemberByName("Falloff")->AsScalar();
	}
}

void Environment::InitializeObjectsEffect(ID3D10Device* d3dDevice, const Camera& camera)
{
	_objectsEffect = ShaderBuilder::RequestEffect("Assets/generic_diffuse_textured", "fx_4_0", d3dDevice);
	_objectsTechnique = _objectsEffect->GetTechniqueByName("Render");
	
	D3D10_PASS_DESC PassDesc;
	_objectsTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	D3D10_INPUT_ELEMENT_DESC layoutObjects[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0},
    };

    UINT numElementsScene = sizeof(layoutObjects) / sizeof(layoutObjects[0]);
	d3dDevice->CreateInputLayout(layoutObjects, numElementsScene, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &_vertexLayoutObjects);

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory(&loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO));
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_BC1_UNORM;

	HRESULT hr;
	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Assets/chest.png", &loadInfo, NULL, &_chestTexture, &hr)))
	{
		// TODO: log an error
	}

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "Assets/treasure.png", &loadInfo, NULL, &_treasureTexture, &hr)))
	{
		// TODO: log an error
	}

	_chestModel.Read("Assets/chest.obj");
	_treasureModel.Read("Assets/chest-treasure.obj");

	_chestModel.Load(d3dDevice);
	_treasureModel.Load(d3dDevice);

	FetchObjectsShaderVariables();

	_objectsShaderVars.Projection->SetMatrix((float*)&camera.GetProjectionMatrix());
}

void Environment::FetchObjectsShaderVariables()
{
	_objectsShaderVars.World = _objectsEffect->GetVariableByName("World")->AsMatrix();
	_objectsShaderVars.View = _objectsEffect->GetVariableByName("View")->AsMatrix();
	_objectsShaderVars.ViewDirection = _objectsEffect->GetVariableByName("ViewDirection")->AsVector();
	_objectsShaderVars.Projection = _objectsEffect->GetVariableByName("Proj")->AsMatrix();
	_objectsShaderVars.Texture = _objectsEffect->GetVariableByName("Texture")->AsShaderResource();

	ID3D10EffectVariable* lights = _objectsEffect->GetVariableByName("Lights");
	int i;
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);
		_objectsShaderVars.Lights.Pos[i] = lighti->GetMemberByName("Position")->AsVector();
		_objectsShaderVars.Lights.Color[i] = lighti->GetMemberByName("Color")->AsVector();
		_objectsShaderVars.Lights.Size[i] = lighti->GetMemberByName("Size")->AsScalar();
		_objectsShaderVars.Lights.Falloff[i] = lighti->GetMemberByName("Falloff")->AsScalar();
	}
}

void Environment::New()
{
	_lights.clear();
	_lightsChanged = true;
	_octaves.clear();
	_shapes.clear();
	_treasureChests.clear();

	Rebuild();
}

void Environment::Unload()
{
	_noiseVolume.Unload();

	_bufferPoint->Release();
	_genSurfaceEffect->Release();
	_vertexLayoutGenSurface->Release();

	_surfaceEffect->Release();
	_vertexLayoutSurface->Release();

	_surfaceTexture->Release();
	_surfaceDisplacement->Release();

	_objectsEffect->Release();
	_vertexLayoutObjects->Release();

	_chestTexture->Release();
	_treasureTexture->Release();

	for(unsigned int i = 0; i < _environmentToDraw.size(); ++i)
		delete _environmentToDraw[i];

	for(unsigned int i = 0; i < _environmentToGenerate.size(); ++i)
		delete _environmentToGenerate[i];
}


// Perform a unit of work in generating the surface, the amount of time that may be spent generating the surface
// is limited to allow the editor to continue rendering smoothly while the GPU generates the surface
void Environment::GenSurface(ID3D10Device* d3dDevice)
{
	unsigned int i = 0;
	int validBlobs = 0;
	for (; i < _shapes.size(); ++i)
	{
		// Ignore any blobs in which any dimension of the scale is 0, to prevent divide by zero errors in effect
		if (_shapes[i].Scale.x != 0 && _shapes[i].Scale.y != 0 && _shapes[i].Scale.z != 0) 
		{	
			_genSurfaceShaderVars.Blobs[validBlobs].Position->SetFloatVector((float*)&_shapes[i].Position);
			_genSurfaceShaderVars.Blobs[validBlobs].Scale->SetFloatVector((float*)&_shapes[i].Scale);
			++validBlobs;
		}
	}

	_genSurfaceShaderVars.NumBlobs->SetInt(validBlobs);

	for (i = 0; i < _octaves.size(); ++i)
	{
		_genSurfaceShaderVars.Octaves[i].Scale->SetFloatVector((float*)&_octaves[i].Scale);
		_genSurfaceShaderVars.Octaves[i].Amplitude->SetFloat(_octaves[i].Amplitude);
	}

	D3DXVECTOR4 defaultScale(0, 0, 0, 0);

	for (; i < MAX_OCTAVES; ++i)
	{
		_genSurfaceShaderVars.Octaves[i].Scale->SetFloatVector((float*)&defaultScale);
		_genSurfaceShaderVars.Octaves[i].Amplitude->SetFloat(0);
	}

	d3dDevice->IASetInputLayout(_vertexLayoutGenSurface);

	UINT offset[1] = {0};
	UINT stride = sizeof(D3DVECTOR);
	d3dDevice->IASetVertexBuffers(0, 1, &_bufferPoint, &stride, offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	Timer t;

	float Budget = 0.001f;

	for (std::vector<EnvironmentChunk*>::iterator i = _environmentToGenerate.begin(); 
		i != _environmentToGenerate.end();)
	{
		EnvironmentChunk* generated = *i;
		t.Start();
		generated->Generate(d3dDevice, _genSurfaceEffect, _genSurfaceTechnique);
		t.Stop();
		Budget -= t.GetTime();
		
		i = _environmentToGenerate.erase(i);

		_environmentToDraw.push_back(generated);

		if (Budget <= 0)
			break;
	}
}

void Environment::Draw(ID3D10Device* d3dDevice, Camera& camera)
{
	if (!_environmentToGenerate.empty())
	{
		SortListToGenerate(camera);
		GenSurface(d3dDevice);
	}

	if (_lightsChanged)
	{
		UpdateLights();
	}
	d3dDevice->IASetInputLayout(_vertexLayoutSurface);

	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX viewm = camera.GetViewMatrix();
	_surfaceShaderVars.View->SetMatrix((float*)&viewm);

	D3DXVECTOR4 viewDirection = D3DXVECTOR4(*(D3DXVECTOR3*)&camera.Look(), 1.0f);
	_surfaceShaderVars.ViewDirection->SetFloatVector((float*)viewDirection);
    _surfaceTechnique->GetPassByIndex(0)->Apply(0);

	for (unsigned int i = 0; i < _environmentToDraw.size(); ++i)
	{
		_environmentToDraw[i]->Draw(d3dDevice, _surfaceEffect);
	}

	d3dDevice->IASetInputLayout(_vertexLayoutObjects);
	
	_objectsShaderVars.View->SetMatrix((float*)&viewm);
	_objectsShaderVars.ViewDirection->SetFloatVector((float*)viewDirection);

	for (unsigned int i = 0; i < _treasureChests.size(); ++i)
	{
		GameObject temp;
		temp.Position = _treasureChests[i].Position + _powerupOffset.Position;
		temp.Rotation = _powerupOffset.Rotation;

		_objectsShaderVars.Texture->SetResource(_chestTexture);
		_objectsShaderVars.World->SetMatrix((float*)&temp.GetMatrix());
		_objectsTechnique->GetPassByIndex(0)->Apply(0);

		_chestModel.Draw(d3dDevice);

		_objectsShaderVars.Texture->SetResource(_treasureTexture);
		_objectsTechnique->GetPassByIndex(0)->Apply(0);

		_treasureModel.Draw(d3dDevice);
	}
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
	for (unsigned int i = 0; i < _environmentToGenerate.size()-1; ++i)
	{
		bool noswap = true;
		for (unsigned int j = 0; j < _environmentToGenerate.size() - 1 - i; ++j)
		{
			if (_environmentToGenerate[j]->Importance(camera) < _environmentToGenerate[j+1]->Importance(camera))
			{
				EnvironmentChunk* temp = _environmentToGenerate[j];
				_environmentToGenerate[j] = _environmentToGenerate[j+1];
				_environmentToGenerate[j+1] = temp;
				noswap = false;
			}
		}

		if (noswap)
			break;
	}
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
	_elapsed += dt;
	_powerupOffset.Position.y = sin(_elapsed) * 0.008f;
	_powerupOffset.Rotation.y += dt;
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
	LoadLightParameters(_surfaceShaderVars.Lights);
	LoadLightParameters(_objectsShaderVars.Lights);
}

void Environment::LoadLightParameters(const LightParam& lightParam)
{
	int i;
	for (i = 0; i < NumLights(); ++i)
	{
		lightParam.Pos[i]->SetFloatVector((float*)&_lights[i]._position);

		D3DXCOLOR col(Util::GetR(_lights[i]._color) / 255.0f,
			Util::GetG(_lights[i]._color) / 255.0f,
			Util::GetB(_lights[i]._color) / 255.0f,
			0);
		lightParam.Color[i]->SetFloatVector(&col.r);
	
		lightParam.Size[i]->SetFloat(_lights[i]._size);
	
		lightParam.Falloff[i]->SetFloat(_lights[i]._falloff);
	}

	for (; i < 8; ++i)
	{
		D3DXCOLOR col(0,0,0,0);
		lightParam.Color[i]->SetFloatVector(&col.r);
	}
}

int Environment::AddOctave()
{
	if (_octaves.size() == MAX_OCTAVES)
		return -1;

	int i = (int)_octaves.size();

	_octaves.push_back(Octave());
	_octaves[i].Scale = D3DXVECTOR4(0,0,0,0);
	_octaves[i].Amplitude = 0;

	return i;
}

void Environment::RemoveOctave(int octave)
{
	_octaves.erase(_octaves.begin() + octave);
}

Vector3f Environment::GetOctaveScale(int octave) const
{
	const D3DXVECTOR4& s = _octaves[octave].Scale;
	return Vector3f(s.x, s.y, s.z);
}

void Environment::SetOctaveScale(int octave, const Vector3f& scale)
{
	_octaves[octave].Scale = D3DXVECTOR4(scale.x, scale.y, scale.z, 0.0f);
}

float Environment::GetOctaveAmplitude(int octave) const
{
	return _octaves[octave].Amplitude;
}

void Environment::SetOctaveAmplitude(int octave, float amplitude)
{
	_octaves[octave].Amplitude = amplitude;
}

int Environment::NumOctaves() const
{
	return (int) _octaves.size();
}

void Environment::RemoveShape(int shape)
{
	_shapes.erase(_shapes.begin() + shape);
}

int Environment::AddShape()
{
	if (_shapes.size() == MAX_BLOBS)
		return -1;

	int i = (int)_shapes.size();

	_shapes.push_back(Blob());
	_shapes[i].Position = D3DXVECTOR4(0,0,0,0);
	_shapes[i].Scale = D3DXVECTOR4(0,0,0,0);

	return i;
}

Vector3f Environment::GetShapePosition(int shape) const
{
	return Vector3f(_shapes[shape].Position.x, _shapes[shape].Position.y, _shapes[shape].Position.z);
}

void Environment::SetShapePosition(int shape, const Vector3f& position)
{
	_shapes[shape].Position.x = position.x;
	_shapes[shape].Position.y = position.y;
	_shapes[shape].Position.z = position.z;
}

Vector3f Environment::GetShapeScale(int shape) const
{
	return Vector3f(_shapes[shape].Scale.x, _shapes[shape].Scale.y, _shapes[shape].Scale.z);
}

void Environment::SetShapeScale(int shape, const Vector3f& scale)
{
	_shapes[shape].Scale.x = scale.x;
	_shapes[shape].Scale.y = scale.y;
	_shapes[shape].Scale.z = scale.z;
}

int Environment::NumShapes() const
{
	return _shapes.size();
}

int Environment::NumChests() const
{
	return _treasureChests.size();
}

Vector3f Environment::GetChestPosition(int chest) const
{
	return _treasureChests[chest].Position;
}

void Environment::SetChestPosition(int chest, const Vector3f& position)
{
	_treasureChests[chest].Position = position;
}

int Environment::AddChest()
{
	int pos = _treasureChests.size();
	_treasureChests.push_back(GameObject());
	return pos;
}

void Environment::RemoveChest(int chest)
{
	_treasureChests.erase(_treasureChests.begin()+chest);
}

bool Environment::Save(const std::wstring& filename) const
{
	std::ofstream file(filename.c_str());

	if (file.is_open())
	{
		file << "version 2\n";
		file << "shapes " << _shapes.size() << "\n";
		file << "{\n";
		for (unsigned int i = 0; i < _shapes.size(); ++i)
		{
			file << _shapes[i].Position.x << ' ';
			file << _shapes[i].Position.y << ' ';
			file << _shapes[i].Position.z << ' ';
			file << _shapes[i].Scale.x << ' ';
			file << _shapes[i].Scale.y << ' ';
			file << _shapes[i].Scale.z << '\n';
		}
		file << "}\n";
		file << "octaves " << _octaves.size() << '\n';
		file << "{\n";
		for (unsigned int i = 0; i < _octaves.size(); ++i)
		{
			file << _octaves[i].Amplitude << ' ';
			file << _octaves[i].Scale.x << ' ';
			file << _octaves[i].Scale.y << ' ';
			file << _octaves[i].Scale.z << '\n';
		}
		file << "}\n";
		file << "lights " << _lights.size() << '\n';
		file << "{\n";
		for (unsigned int i = 0; i < _lights.size(); ++i)
		{
			file << _lights[i]._position.x << ' ';
			file << _lights[i]._position.y << ' ';
			file << _lights[i]._position.z << ' ';
			file << _lights[i]._size << ' ';
			file << _lights[i]._falloff << ' ';
			file << std::hex << std::left << std::setw(2) << std::setfill('0') << Util::GetR(_lights[i]._color);
			file << std::hex << std::left << std::setw(2) << std::setfill('0') << Util::GetG(_lights[i]._color);
			file << std::hex << std::left << std::setw(2) << std::setfill('0') << Util::GetB(_lights[i]._color) << '\n';
		}
		file << "}\n";
		file << "chests " << _treasureChests.size() << '\n';
		file << "{\n";
		for (unsigned int i = 0; i < _treasureChests.size(); ++i)
		{
			file << _treasureChests[i].Position.x << ' ';
			file << _treasureChests[i].Position.y << ' ';
			file << _treasureChests[i].Position.z << '\n';
		}
		file << "}\n";

		file.close();

		return !file.fail();
	}
	else
	{
		return false;
	}
}

bool Environment::Open(const std::wstring& filename)
{

	std::ifstream file(filename.c_str());

	bool error = false;

	if (file.is_open())
	{
		std::string token;

		file >> token;
		if (token != "version") error = true;

		int version = 0;
		file >> version;
		if (version != 2) error = true;

		file >> token;
		if (token != "shapes") error = true;

		int numShapes = -1;
		file >> numShapes;
		if (numShapes < 0) error = true;

		std::vector<Blob> tempShapes(numShapes);

		file >> token;
		if (token != "{") error = true;

		for (int i = 0; i < numShapes; ++i)
		{
			file >> tempShapes[i].Position.x;
			file >> tempShapes[i].Position.y;
			file >> tempShapes[i].Position.z;

			file >> tempShapes[i].Scale.x;
			file >> tempShapes[i].Scale.y;
			file >> tempShapes[i].Scale.z;
		}

		if (file.fail()) error = true;
		
		file >> token;
		if (token != "}") error = true;

		file >> token;
		if (token != "octaves") error = true;

		int numOctaves = -1;
		file >> numOctaves;
		if (numOctaves < 0) error = true;

		std::vector<Octave> tempOctaves(numOctaves);

		file >> token;
		if (token != "{") error = true;

		for (int i = 0; i < numOctaves; ++i)
		{
			file >> tempOctaves[i].Amplitude;
			file >> tempOctaves[i].Scale.x;
			file >> tempOctaves[i].Scale.y;
			file >> tempOctaves[i].Scale.z;
		}

		file >> token;
		if (token != "}") error = true;

		file >> token;
		if (token != "lights") error = true;

		int numLights = -1;
		file >> numLights;
		if (numLights < 0) error = true;

		std::vector<Light> tempLights(numLights);

		file >> token;
		if (token != "{") error = true;

		for (int i = 0; i < numLights; ++i)
		{
			file >> tempLights[i]._position.x;
			file >> tempLights[i]._position.y;
			file >> tempLights[i]._position.z;

			file >> tempLights[i]._size;
			file >> tempLights[i]._falloff;

			if (!Util::ReadHexColor(file, tempLights[i]._color))
				error = true;
		}

		file >> token;
		if (token != "}") error = true;

		file >> token;
		if (token != "chests") error = true;
		
		int numChests = -1;
		file >> numChests;
		if (numChests < 0) error = true;
		std::vector<GameObject> tempChests(numChests);

		file >> token;
		if (token != "{") error = true;

		for (int i = 0; i < numChests; ++i)
		{
			file >> tempChests[i].Position.x;
			file >> tempChests[i].Position.y;
			file >> tempChests[i].Position.z;
		}

		file>>token;
		if (token != "}") error = true;

		file.close();

		if (file.fail()) error = true;

		if (!error)
		{
			// loading code
			_lights = tempLights;
			_lightsChanged = true;

			_shapes = tempShapes;
			_octaves = tempOctaves;
			_treasureChests = tempChests;

			Rebuild();
		}
	}
	else
	{
		error = true;
	}

	return !error;
}

float Environment::Sample(const Vector3f& position)
{
	float density = 0;
	for (unsigned int i = 0; i < _shapes.size(); ++i)
	{
		Vector3f shapepos (_shapes[i].Position.x, _shapes[i].Position.y, _shapes[i].Position.z);
		Vector3f shapeScale (_shapes[i].Scale.x,  _shapes[i].Scale.y,  _shapes[i].Scale.z);
		density += 1.0f / (((position - shapepos)/ shapeScale).Length() + 0.0001f);
	}

	for (unsigned int i = 0; i < _octaves.size(); ++i)
	{
		float v = _noiseVolume.Sample(position * Vector3f(_octaves[i].Scale.x, _octaves[i].Scale.y, _octaves[i].Scale.z) / 64) * 2.0f - 1.0f;
		density += v * _octaves[i].Amplitude;
    }

	return density;
}

Vector3f Environment::SampleNormal(const Vector3f& position)
{
	float CubeSize = 1/64.0f;
	Vector3f d ((Sample(position+Vector3f(CubeSize, 0.0f, 0.0f))-3.6f) - (Sample(position - Vector3f(CubeSize, 0.0f, 0.0f))-3.6f),
			    (Sample(position+Vector3f(0.0f, CubeSize, 0.0f))-3.6f) - (Sample(position - Vector3f(0.0f, CubeSize, 0.0f))-3.6f),
				(Sample(position+Vector3f(0.0f, 0.0f, CubeSize))-3.6f) - (Sample(position - Vector3f(0.0f, 0.0f, CubeSize))-3.6f));

	d /= CubeSize*2;

	return d;
}

void Environment::Reset()
{
	_elapsed = 0;
	_powerupOffset.Position = Vector3f(0, 0, 0);
	_powerupOffset.Rotation = Vector3f(0, 0, 0);
}