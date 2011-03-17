cbuffer cbWorldMatrix
{
	float4x4 World: World;
};

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
	Light lights[8];
};

Texture2D tex;
Texture2D texDisplacement;

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
};

PS_INPUT mainVS(VS_INPUT input)
{
	float4x4 WorldViewProj = mul(World, mul(View, Proj));
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
	float3 blendWeights = abs(iNormal);

	float stepSize = 1/256.0f;
	float heightMapScale = 2;

	float3 textureCoord = input.WSPos * 5.0f;

	float3 nx = float3(1,
					   texDisplacement.Sample(TextureSampler, textureCoord.zy+float2(0, stepSize)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.zy-float2(0, stepSize)).r*heightMapScale,
					   texDisplacement.Sample(TextureSampler, textureCoord.zy+float2(stepSize, 0)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.zy-float2(stepSize, 0)).r*heightMapScale);

	float3 ny = float3(texDisplacement.Sample(TextureSampler, textureCoord.xz+float2(0, stepSize)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.xz-float2(0, stepSize)).r*heightMapScale,
					   1,
					   texDisplacement.Sample(TextureSampler, textureCoord.xz+float2(stepSize, 0)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.xz-float2(stepSize, 0)).r*heightMapScale);

	float3 nz = float3(texDisplacement.Sample(TextureSampler, textureCoord.xy+float2(0, stepSize)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.xy-float2(0, stepSize)).r*heightMapScale,
					   texDisplacement.Sample(TextureSampler, textureCoord.xy+float2(stepSize, 0)).r*heightMapScale-
					   texDisplacement.Sample(TextureSampler, textureCoord.xy-float2(stepSize, 0)).r*heightMapScale,
					   1);

	float3 N = normalize ((nz * iNormal.z + 
						ny * iNormal.y + 
						nx * iNormal.x));

	float3 spec = float3(0,0,0);
	float3 diffuse = float3(0,0,0);
	
	float4 ambient = float4(0.08f, 0.08f, 0.08f, 1.0f);

	[unroll] for (int i = 0; i < 8; ++i)
	{
		float3 lightDirection = lights[i].Position - input.WSPos;
		float attenuation = clamp(lights[i].Size/(length(lightDirection) + pow(length(lightDirection), 2.0f)*lights[i].Falloff), 0.0f, 1.0f);
		diffuse += max(dot(N, normalize(lightDirection)),0) * attenuation * lights[i].Color.rgb;
		spec += pow(clamp(dot(reflect(ViewDirection, N), normalize(lightDirection)), 0.0f, 1.0f),22.0f) * attenuation*2.0f * lights[i].Color.rgb * 0.2f;
	}

	float4 s1 = tex.Sample(TextureSampler, textureCoord.xy) * blendWeights.z;
	float4 s2 = tex.Sample(TextureSampler, textureCoord.xz) * blendWeights.y;
	float4 s3 = tex.Sample(TextureSampler, textureCoord.zy) * blendWeights.x;

	float4 diffuseCol = s1+s2+s3;

	return float4((ambient+diffuse+spec*0.3f)*diffuseCol.rgb, diffuseCol.a);
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