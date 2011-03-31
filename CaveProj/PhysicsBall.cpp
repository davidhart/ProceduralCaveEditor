#include "PhysicsBall.h"
#include "Environment.h"
#include "RenderWindow.h"
#include "ShaderBuilder.h"
#include <cstdlib>

PhysicsBall::PhysicsBall(Environment& environment) :
	_environment(environment),
	_radius(0.025f),
	_position(0, 0, 0),
	_velocity(0, 0, 0),
	_inside(false)
{

	// TODO: statically initialise these
	std::srand(1231);

	for (int i = 0; i < 16; ++i)
	{
		Vector3f v ((std::rand() % 1000) / 1000.0f,
			(std::rand() % 1000) / 1000.0f,
			(std::rand() % 1000) / 1000.0f);

		v = v.Normalize();

		v.x *= std::rand() % 2 * 2 - 1;
		v.y *= std::rand() % 2 * 2 - 1;
		v.z *= std::rand() % 2 * 2 - 1;

		_samplePositions.push_back(v * _radius);
		_samplePositions.push_back(v * -_radius);
	}

	_ball.Read("Sphere.obj");
}

void PhysicsBall::Load(RenderWindow& renderWindow)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();

	_ball.Load(d3dDevice);

	_genericEffect = ShaderBuilder::RequestEffect("generic_diffuse", "fx_4_0", d3dDevice);
	_genericTechnique = _genericEffect->GetTechniqueByName("Render");

	_view = _genericEffect->GetVariableByName("View")->AsMatrix();
	_proj = _genericEffect->GetVariableByName("Proj")->AsMatrix();
	_world = _genericEffect->GetVariableByName("World")->AsMatrix();
	_viewDirection = _genericEffect->GetVariableByName("ViewDirection")->AsVector();

	D3D10_PASS_DESC PassDesc;
	_genericTechnique->GetPassByIndex( 0 )->GetDesc(&PassDesc);

	D3D10_INPUT_ELEMENT_DESC layoutRender[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };

	UINT numElementsRender = sizeof( layoutRender ) / sizeof( layoutRender[0] );
	d3dDevice->CreateInputLayout( layoutRender, numElementsRender, PassDesc.pIAInputSignature,
                                  PassDesc.IAInputSignatureSize, &_vertexLayout );
	
	ID3D10EffectVariable* lights = _genericEffect->GetVariableByName("lights");
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

void PhysicsBall::Reset()
{
	_position = Vector3f(0, 0, 0);
	_velocity = Vector3f(0, 0, 0);

	_environment.LoadLightParameters(_lightParams);
}

void PhysicsBall::Unload()
{
	_ball.Unload();
}

void PhysicsBall::Draw(RenderWindow& renderWindow, const Camera& camera)
{
	ID3D10Device* d3dDevice = renderWindow.GetDevice();
	d3dDevice->IASetInputLayout(_vertexLayout);

	D3DXMATRIX worldm;
	D3DXMATRIX scale; 
	D3DXMATRIX translate;
	D3DXMatrixScaling(&scale, _radius, _radius, _radius);
	D3DXMatrixTranslation(&translate, _position.x, _position.y, _position.z);
	D3DXMatrixMultiply(&worldm, &scale, &translate);
	_world->SetMatrix((float*)&worldm);

	D3DXMATRIX viewm = camera.GetViewMatrix();
	_view->SetMatrix((float*)&viewm);

	_proj->SetMatrix((float*)&camera.GetProjectionMatrix());

	D3DXVECTOR4 viewDirection = D3DXVECTOR4(*(D3DXVECTOR3*)&camera.Look(), 1.0f);
	_viewDirection->SetFloatVector((float*)viewDirection);

	_genericTechnique->GetPassByIndex(0)->Apply(0);
	_ball.Draw(d3dDevice);
}

void PhysicsBall::Update(float dt)
{
	const float maxTimeStep = 1 / 250.0f;

	while (dt > maxTimeStep)
	{
		UpdateStep(maxTimeStep);
		dt -= maxTimeStep;
	}

	UpdateStep(dt);
}

void PhysicsBall::UpdateStep(float dt)
{
	Vector3f prevPos = _position;
	_position += _velocity * dt;
	_velocity += Vector3f(0, -0.005f, 0);
	_velocity -= _velocity * 0.001f;

	Vector3f normal;
	bool collision = false;
	for (unsigned int i = 0; i < _samplePositions.size(); ++i)
	{
		if (_environment.Sample(_position + _samplePositions[i]) < 3.6f)
		{
			normal+= _environment.SampleNormal(_position + _samplePositions[i]);
			collision = true;
		}
	}

	if (collision)
	{
		_position = prevPos;
		normal = normal.Normalize();
		_velocity -= 2 * _velocity.Dot(normal) * normal;
		_velocity *= 0.80f;
	}
}