
cbuffer ViewMatrix
{
	float4x4 View : View;
	float3 ViewDirection;
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

Texture2D tex;
Texture2D tex2;
Texture2D texBump;

sampler TextureSampler = sampler_state
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
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
	float3 LightDirection : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
	float4x4 WorldViewProj = mul(World, mul(View, Proj));
    PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1), WorldViewProj);
	output.WSPos = input.Pos;
	output.Normal = input.Normal;
	output.LightDirection = LightPosition.xyz - input.Pos;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{	
	
	float3x3 NBT = float3x3 (input.Normal.zyx * float3(1,1,1),
		input.Normal.xzy * float3(1,1,1),
		input.Normal);

	float3 lightDirection = input.LightDirection;

	float3 iNormal = normalize(input.Normal);

	float3 blendWeights = abs(normalize(iNormal));

	float3 b1 = texBump.Sample(TextureSampler, input.WSPos.yx*3.0f).rgb-0.5f;
	float3 b2 = texBump.Sample(TextureSampler, input.WSPos.zx*3.0f).rgb-0.5f;
	float3 b3 = texBump.Sample(TextureSampler, input.WSPos.yz*3.0f).rgb-0.5f;

	float3 bump1 = float3(b1.x, b1.y, 0) * blendWeights.z;
	float3 bump2 = float3(b2.x, 0, b2.z) * blendWeights.y;
	float3 bump3 = float3(0, b3.y, b3.z) * blendWeights.x;

	float3 N = normalize(input.Normal + (bump1+bump2+bump3) * 1.3f);

	float attenuation = clamp(1.0f/(length(lightDirection) + pow(length(lightDirection), 2.0f)*5.0f), 0.0f, 1.0f);
	float diffuseAmt = max(dot(N, normalize(lightDirection)),0) * attenuation;
	float4 diffuse = float4(float3(0.7f, 0.65f, 0.65f)*diffuseAmt, 1.0f);

	float specAmt = pow(clamp(dot(reflect(ViewDirection, N), normalize(lightDirection)), 0.0f, 1.0f),22.0f) * attenuation*2.0f;
	float4 spec = float4(float3(0.15f, 0.15f, 0.15f)*specAmt, 1.0f);
	
	float4 ambient = float4(0.05f, 0.05f, 0.05f, 1.0f);

	float4 s1 = tex.Sample(TextureSampler, input.WSPos.xy*5.0f) * blendWeights.z;
	float4 s2 = tex.Sample(TextureSampler, input.WSPos.xz*6.0f) * blendWeights.y;
	float4 s3 = tex.Sample(TextureSampler, input.WSPos.zy*5.0f) * blendWeights.x;

	float4 diffuseCol = s1+s2+s2+s3;

	return (ambient+diffuse+spec)*diffuseCol;
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