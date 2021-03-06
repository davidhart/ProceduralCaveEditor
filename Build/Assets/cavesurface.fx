cbuffer cbViewMatrix
{
	float4x4 View : View;
	float3 ViewDirection;
};

cbuffer cbProjectionMatrix
{
	float4x4 Proj: Projection;
};

struct Light
{
	float3 Position;
	float4 Color;
	float Size;
	float Falloff;
};

cbuffer cbLight
{
	Light Lights[8];
};

Texture2D Texture;
Texture2D Displacement;

sampler TextureSampler = sampler_state
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
	AddressW = Wrap;
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
	float4x4 WorldViewProj = mul(View, Proj);
    PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1), WorldViewProj);
	output.WSPos = input.Pos;
	output.Normal = input.Normal;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
	float3x3 NBT = float3x3 (input.Normal.zyx * float3(1,1,1),
		input.Normal.xzy * float3(1,1,1),
		input.Normal);

	float3 iNormal = normalize(input.Normal);
	float3 absNormal = abs(iNormal);

	float3 blendWeights = iNormal / (absNormal.x + absNormal.y + absNormal.z);
	float3 absBlendWeights = abs(blendWeights);

	float stepSize = 1/512.0f;
	float heightMapScale = 3;

	float3 textureCoord = input.WSPos * 5.0f + float3(0.3f, 0.546f, -0.1514f);

	float3 nx = float3(1,
					   Displacement.Sample(TextureSampler, textureCoord.zy+float2(0, stepSize)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.zy-float2(0, stepSize)).r*heightMapScale,
					   Displacement.Sample(TextureSampler, textureCoord.zy+float2(stepSize, 0)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.zy-float2(stepSize, 0)).r*heightMapScale);

	float3 ny = float3(Displacement.Sample(TextureSampler, textureCoord.xz+float2(0, stepSize)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.xz-float2(0, stepSize)).r*heightMapScale,
					   1,
					   Displacement.Sample(TextureSampler, textureCoord.xz+float2(stepSize, 0)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.xz-float2(stepSize, 0)).r*heightMapScale);

	float3 nz = float3(Displacement.Sample(TextureSampler, textureCoord.xy+float2(0, stepSize)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.xy-float2(0, stepSize)).r*heightMapScale,
					   Displacement.Sample(TextureSampler, textureCoord.xy+float2(stepSize, 0)).r*heightMapScale-
					   Displacement.Sample(TextureSampler, textureCoord.xy-float2(stepSize, 0)).r*heightMapScale,
					   1);

	float3 N = normalize(nz * blendWeights.z + 
						ny *  blendWeights.y + 
						nx *  blendWeights.x);

	float3 spec = float3(0,0,0);
	float3 diffuse = float3(0,0,0);
	
	float4 ambient = float4(0.13f, 0.13f, 0.13f, 1.0f);

	[unroll] for (int i = 0; i < 8; ++i)
	{
		float3 lightDirection = Lights[i].Position - input.WSPos;
		float attenuation = clamp(Lights[i].Size/(length(lightDirection) + pow(length(lightDirection), 2.0f)*Lights[i].Falloff), 0.0f, 1.0f);
		diffuse += max(dot(N, normalize(lightDirection)),0) * attenuation * Lights[i].Color.rgb;
		spec += pow(clamp(dot(reflect(ViewDirection, N), normalize(lightDirection)), 0.0f, 1.0f),22.0f) * attenuation*2.0f * Lights[i].Color.rgb * 0.2f;
	}

	float4 s1 = Texture.Sample(TextureSampler, textureCoord.xy) * absBlendWeights.z;
	float4 s2 = Texture.Sample(TextureSampler, textureCoord.xz) * absBlendWeights.y;
	float4 s3 = Texture.Sample(TextureSampler, textureCoord.zy) * absBlendWeights.x;

	float4 diffuseCol = s1+s2+s3;

	return float4((clamp(ambient+diffuse, 0, 1)+spec*0.3f)*diffuseCol.rgb, diffuseCol.a);
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