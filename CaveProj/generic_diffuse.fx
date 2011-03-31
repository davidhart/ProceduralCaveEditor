cbuffer ViewMatrix
{
	float4x4 View : View;
	float3 ViewDirection;
}

cbuffer WorldMatrix
{
	float4x4 World: World;
}

cbuffer ProjectionMatrix
{
	float4x4 Proj: Projection;
}

struct Light
{
	float3 Position;
	float4 Color;
	float Size;
	float Falloff;
};

cbuffer cbLight
{
	Light lights[8];
};

struct VS_INPUT
{
    float3 Pos          : POSITION;
	float3 Normal		: NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float3 WSPos : POSITION;
	float3 Normal : NORMAL;
};

PS_INPUT mainVS(VS_INPUT input)
{
	float4x4 WorldViewProj = mul(World, mul(View, Proj));
    PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1), WorldViewProj);
	output.WSPos = mul(float4(input.Pos, 1), World).xyz;
	output.Normal = input.Normal;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{	
	float3 spec = float3(0,0,0);
	float3 diffuse = float3(0,0,0);
	
	float4 ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);

	float3 N = normalize(input.Normal);

	[unroll] for (int i = 0; i < 8; ++i)
	{
		float3 lightDirection = lights[i].Position - input.WSPos;
		float attenuation = clamp(lights[i].Size * 2/(length(lightDirection) + pow(length(lightDirection), 2.0f)*lights[i].Falloff), 0.0f, 1.0f);
		diffuse += max(dot(N, normalize(lightDirection)),0) * attenuation * lights[i].Color.rgb;
		spec += pow(clamp(dot(reflect(ViewDirection, N), normalize(lightDirection)), 0.0f, 1.0f),15.0f) * attenuation*2.0f * lights[i].Color.rgb * 0.2f;
	}

	return float4(spec + diffuse, 0) + ambient;
}

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
    //DepthEnable = FALSE;
    //DepthWriteMask = 0;
};


RasterizerState RasterizerSettings
{
    CullMode = BACK;
};

technique10 Render
{
	pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, mainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, mainPS()));
		
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(RasterizerSettings); 
	}
}