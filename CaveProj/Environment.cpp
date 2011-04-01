#include "Environment.h"
#include "RenderWindow.h"

#include "Camera.h"
#include "ShaderBuilder.h"
#include "MarchingCubesData.h"
#include "Timer.h"
#include "Util.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
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
	_noiseVolume(Vector3i(256, 256, 256))
{

	int o = AddOctave();
	SetOctaveScale(o, Vector3f(1.5f, 1.5f, 1.5f));
	SetOctaveAmplitude(o, 0.5f);

	o = AddOctave();
	SetOctaveScale(o, Vector3f(3.25f, 3.25f, 3.25f));
	SetOctaveAmplitude(o, 0.1f);

	o = AddOctave();
	SetOctaveScale(o, Vector3f(5.0f, 0.025f, 5.0f));
	SetOctaveAmplitude(o, 0.4f);
}

void Environment::GenBlobs()
{
	_shapes.resize(MAX_BLOBS);

	std::srand(152);

	for (int i = 0; i < MAX_BLOBS; ++i)
	{
		_shapes[i].Position = D3DXVECTOR4(std::rand() % 200 / 100.0f - 1.0f,
			std::rand() % 200 / 100.0f - 1.0f,
			std::rand() % 200 / 100.0f - 1.0f,
			0.0f);

		float r = std::rand() % 20 / 24.0f + 0.4f;
		_shapes[i].Scale = D3DXVECTOR4(r, r, r, 0);
	}

	std::cout << "Generated blobs" << std::endl;
}

void Environment::GenModel(ID3D10Device* d3dDevice)
{
	// TODO: Store the variable locations at load and validate
	ID3D10EffectVariable* blobs = _genModelEffect->GetVariableByName("blobs");
	
	unsigned int i = 0;
	int validBlobs = 0;
	for (; i < _shapes.size(); ++i)
	{
		if (_shapes[i].Scale.x != 0 && _shapes[i].Scale.y != 0 && _shapes[i].Scale.z != 0)
		{
			ID3D10EffectVariable* blobi = blobs->GetElement(validBlobs);		
			blobi->GetMemberByName("Position")->AsVector()->SetFloatVector((float*)&_shapes[i].Position);
			blobi->GetMemberByName("Scale")->AsVector()->SetFloatVector((float*)&_shapes[i].Scale);
			++validBlobs;
		}
	}

	_genModelEffect->GetVariableByName("NumBlobs")->AsScalar()->SetInt(validBlobs);

	ID3D10EffectVariable* octaves = _genModelEffect->GetVariableByName("octaves");
	for (i = 0; i < _octaves.size(); ++i)
	{
		ID3D10EffectVariable* octavei = octaves->GetElement(i);
		octavei->GetMemberByName("Scale")->AsVector()->SetFloatVector((float*)&_octaves[i].Scale);
		octavei->GetMemberByName("Amplitude")->AsScalar()->SetFloat(_octaves[i].Amplitude);
	}

	for (; i < MAX_OCTAVES; ++i)
	{
		ID3D10EffectVariable* octavei = octaves->GetElement(i);
		D3DXVECTOR4 s(0, 0, 0, 0);
		octavei->GetMemberByName("Scale")->AsVector()->SetFloatVector((float*)&s);
		octavei->GetMemberByName("Amplitude")->AsScalar()->SetFloat(0);
	}

	d3dDevice->IASetInputLayout(_vertexLayoutGen);

	UINT offset[1] = {0};
	UINT stride = sizeof(D3DVECTOR);
	d3dDevice->IASetVertexBuffers(0, 1, &_bufferPointGrid, &stride, offset);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	Timer t;

	float Budget = 0.001f; // limit the amount of time we can spend per frame generating surface chunks
						   // when the limit is exceeded, stop generating and render the frame to keep
						   // the application responsive

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
	_noiseVolume.Load(d3dDevice);

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

	std::cout << "Created textures" << std::endl;

	// Create shader and get render technique
	_renderSceneEffect = ShaderBuilder::RequestEffect("cavesurface", "fx_4_0", d3dDevice);

	_renderSceneTechnique = _renderSceneEffect->GetTechniqueByName("Render");

	_genModelEffect = ShaderBuilder::RequestEffect("marchingcubes", "fx_4_0", d3dDevice);

	_genModelTechnique = _genModelEffect->GetTechniqueByName("Render");


	_genModelEffect->GetVariableByName("NoiseTexture")->AsShaderResource()->SetResource(_noiseVolume.GetResource());

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

	ID3D10EffectScalarVariable* threshold = _genModelEffect->GetVariableByName("Threshold")->AsScalar();
	threshold->SetFloat(3.6f);

	ID3D10EffectShaderResourceVariable* texturesampler = _renderSceneEffect->GetVariableByName("tex")->AsShaderResource();
	texturesampler->SetResource(_texture);

	texturesampler = _renderSceneEffect->GetVariableByName("texDisplacement")->AsShaderResource();
	texturesampler->SetResource(_textureDisplacement);


	ID3D10EffectVariable* lights = _renderSceneEffect->GetVariableByName("lights");
	int i;
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);
		_surfaceLightParam.Pos[i] = lighti->GetMemberByName("Position")->AsVector();
		_surfaceLightParam.Color[i] = lighti->GetMemberByName("Color")->AsVector();
		_surfaceLightParam.Size[i] = lighti->GetMemberByName("Size")->AsScalar();
		_surfaceLightParam.Falloff[i] = lighti->GetMemberByName("Falloff")->AsScalar();
	}
	for (float x = -2; x < 2; x += 1)
		for (float y = -2; y < 2; y += 1)
			for (float z = -2; z < 2; z += 1)
				_environmentToGenerate.push_back(new EnvironmentChunk(Vector3f(x, y, z), 1, 32));

	NewCave(d3dDevice);

	AddLight();
}

void Environment::NewCave(ID3D10Device* d3dDevice)
{
	GenBlobs();
}

void Environment::Unload()
{
	_noiseVolume.Unload();

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
	LoadLightParameters(_surfaceLightParam);

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

bool Environment::Save(const std::wstring& filename) const
{
	std::ofstream file(filename.c_str());

	if (file.is_open())
	{
		file << "version 1\n";
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
		file << "}";

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
		if (version != 1) error = true;

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
		file >>numLights;
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

		file >>token;
		if (token != "}") error = true;

		file.close();

		if (!error)
		{
			// loading code
			_lights = tempLights;
			_lightsChanged = true;

			_shapes = tempShapes;
			_octaves = tempOctaves;

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