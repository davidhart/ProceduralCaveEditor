
cbuffer ViewMatrix
{
	float4x4 View : View;
}
cbuffer Light
{
	float4 LightPosition;
}

cbuffer WorldMatrix
{
	float4x4 World: World;
}
cbuffer ProjectionMatrix
{
	float4x4 Proj: Projection;
}

struct VS_INPUT
{
    float3 Pos          : POSITION;
	float3 Normal		: NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float3 Normal : NORMAL;
	float3 LightDirection : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
	float4x4 WorldViewProj = mul(World, mul(View, Proj));
    PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1), WorldViewProj);
	output.Normal = input.Normal;
	output.LightDirection = LightPosition.xyz - input.Pos;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{	
	
	float diffuseAmt = max(dot(normalize(input.Normal), normalize(input.LightDirection)),0);
	float4 diffuse = float4(float3(0.7f, 0.7f, 0.7f)*diffuseAmt, 1.0f);
	
	float4 ambient = float4(0.08f, 0.08f, 0.08f, 1.0f);
	
	return ambient+diffuse;
}

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

RasterizerState RasterizerSettings
{
	//FillMode = WIREFRAME;
    CullMode = BACK;
};

technique10 Render
{
	pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, mainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, mainPS()));
		
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(RasterizerSettings); 
	}
}