#include "ParticleSystem.h"
#include "RenderWindow.h"
#include "Util.h"
#include "ShaderBuilder.h"

ParticleSystem::ParticleSystem(unsigned int maxParticles, Environment& environment) :
	_particles(maxParticles),
	_environment(environment)
{
	for (unsigned int i = 0; i < _particles.size(); ++i)
	{
		SpawnParticle(i);
	}
}

void ParticleSystem::SpawnParticle(unsigned int i)
{
	_particles[i]._position = Vector3f(0, 0, (Util::Rand.Next() % 10000) * 0.5f / 10000.0f - 0.25f);


	Vector3f v ((Util::Rand.Next() % 1000) / 5000.0f - 0.1f,
		-1.0f,
		(Util::Rand.Next() % 1000) / 5000.0f - 0.1f);

	v.Normalize();

	_particles[i]._velocity = v*0.3f;

	_particles[i]._lifespan = 2 + Util::Rand.Next() % 2000 / 1000.0f;
	_particles[i]._aliveTime = 0;
}

void ParticleSystem::Load(RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	D3DX10_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX10_IMAGE_LOAD_INFO) );
	loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	HRESULT hr;

	if (FAILED(D3DX10CreateShaderResourceViewFromFile(d3dDevice, "WaterParticle.png", &loadInfo, NULL, &_particleTexture, &hr)))
	{
		MessageBox(0, "Error creating texture", "Texture Error", MB_OK);
	}

	_particleEffect = ShaderBuilder::RequestEffect("particle_system", "fx_4_0", d3dDevice);
	_particleRenderTechnique = _particleEffect->GetTechniqueByName("Render");

	_worldviewprojection = _particleEffect->GetVariableByName("WorldViewProjection")->AsMatrix();

	D3D10_PASS_DESC PassDesc;
	_particleRenderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	D3D10_INPUT_ELEMENT_DESC layoutRender[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	UINT numElements = sizeof(layoutRender) / sizeof(layoutRender[0]);
	d3dDevice->CreateInputLayout(layoutRender, numElements, PassDesc.pIAInputSignature,
                                  PassDesc.IAInputSignatureSize, &_vertexLayout);

	_particleEffect->GetVariableByName("Tex")->AsShaderResource()->SetResource(_particleTexture);
	_invview = _particleEffect->GetVariableByName("InvView")->AsMatrix();

	ID3D10EffectVariable* lights = _particleEffect->GetVariableByName("lights");
	int i;
	for (i = 0; i < Environment::MAX_LIGHTS; ++i)
	{
		ID3D10EffectVariable* lighti = lights->GetElement(i);
		_lightParams.Pos[i] = lighti->GetMemberByName("Position")->AsVector();
		_lightParams.Color[i] = lighti->GetMemberByName("Color")->AsVector();
		_lightParams.Size[i] = lighti->GetMemberByName("Size")->AsScalar();
		_lightParams.Falloff[i] = lighti->GetMemberByName("Falloff")->AsScalar();
	}
	_environment.LoadLightParameters(_lightParams);
}

void ParticleSystem::Unload()
{
	_particleTexture->Release();
	_particleEffect->Release();
	_vertexLayout->Release();
}

void ParticleSystem::Draw(RenderWindow& renderWindow, const Camera& camera)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	D3DXMATRIX viewm = camera.GetViewMatrix();
	D3DXMATRIX worldviewproj;
	D3DXMatrixMultiply(&worldviewproj, &viewm, &camera.GetProjectionMatrix());
	_worldviewprojection->SetMatrix((float*)&worldviewproj);

	D3DXMATRIX invview;
	D3DXMatrixInverse(&invview, NULL, &viewm);
	_invview->SetMatrix((float*)&invview);

	_particleRenderTechnique->GetPassByIndex(0)->Apply(0);

	d3dDevice->IASetInputLayout(_vertexLayout);
	d3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = _particles.size() *sizeof(Particle);
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &_particles[0];

	ID3D10Buffer* buffer;
	d3dDevice->CreateBuffer(&bd, &InitData, &buffer);

	UINT stride = sizeof(Particle);
	UINT offset = 0;
	d3dDevice->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	d3dDevice->Draw(_particles.size(), 0);

	ID3D10Buffer* null = NULL;
	d3dDevice->IASetVertexBuffers(0, 1, &null, &stride, &offset); // Suppress "currently bound buffer is released" warnings

	buffer->Release();
}

void ParticleSystem::Update(float dt)
{
	int steps = 0;
	while (dt > 1 / 60.0f && steps < 4)
	{
		dt -= 1 / 60.0f;
		UpdateStep(1 / 60.0f);
		++steps;
	}

	if (dt > 0 && steps < 4)
		UpdateStep(dt);
}

void ParticleSystem::UpdateStep(float dt)
{
	for (unsigned int i = 0; i < _particles.size(); ++i)
	{
		_particles[i]._aliveTime += dt;

		if (_particles[i]._aliveTime >= _particles[i]._lifespan)
			SpawnParticle(i);

		_particles[i].Update(dt, _environment);
	}
}

void ParticleSystem::Reset()
{
	_environment.LoadLightParameters(_lightParams);
}