cbuffer Matrices
{
	float4x4 WorldViewProjection;
	float4x4 InvView;
};

cbuffer ParticleSystem
{
	float3 Center;
};

cbuffer Immutable
{
    float3 VertPos[4] =
    {
        float3(-0.5, 0.5, 0),
        float3(0.5, 0.5, 0),
        float3(-0.5, -0.5, 0),
        float3(0.5, -0.5, 0),
    };

    float2 VertTexCoord[4] = 
    { 
        float2(0,0), 
        float2(1,0),
        float2(0,1),
        float2(1,1),
    };
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

Texture2D Tex;

sampler TextureSampler = sampler_state
{
 	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct GS_INPUT
{
	float3 Pos : POSITION;
	float3 Vel : TEXCOORD0;
	float Life : TEXCOORD1;
	float Alive : TEXCOORD2;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord: TEXCOORD0;
	float4 Color : COLOR0;
	float3 WSPos : POSITION;
};

GS_INPUT mainVS(GS_INPUT input)
{
    return input;
}

[maxvertexcount(4)]
void mainGS(point GS_INPUT input[1],
        inout TriangleStream<PS_INPUT> SpriteStream)
{
	float lifeFrac = 1 - input[0].Alive / input[0].Life;

	float len = 0.1f * clamp(abs(input[0].Vel.y), 0.4f, 1.8f);
	float width = 0.025f;

	float3 size = float3(width, len, 1);

	PS_INPUT output;
    [unroll] for(int i = 0; i < 4; i++)
    {
        float3 position = VertPos[i] * size;
		position = mul(position, (float3x3)InvView) + input[0].Pos;
		output.WSPos = input[0].Pos;
        output.Pos = mul(float4(position, 1.0), WorldViewProjection);
        
        output.Color = float4(1, 1, 1, lifeFrac);
        output.TexCoord = VertTexCoord[i];
        SpriteStream.Append(output);
    }
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{	
	float4 color = input.Color * Tex.Sample(TextureSampler, input.TexCoord);
	
	if (color.a <= 0.0f)
		discard;

	float3 diffuse = float3(0, 0, 0);
	float3 ambient = float3(0.2f, 0.2f, 0.2f);
	float TA = 0.5f;
	[unroll] for (int i = 0; i < 8; ++i)
	{
		float3 lightDirection = lights[i].Position - input.WSPos;
		float attenuation = clamp(lights[i].Size/(length(lightDirection) + pow(length(lightDirection), 2.0f)*lights[i].Falloff), 0.0f, 1.0f);
		TA += attenuation;
		diffuse += attenuation * lights[i].Color.rgb;
	}

	return min(float4(diffuse + ambient, TA), 1) * color;
}

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};

RasterizerState RasterizerSettings
{
	//FillMode = WIREFRAME;
    CullMode = NONE;
};

technique10 Render
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_4_0, mainVS()));
		SetGeometryShader(CompileShader(gs_4_0, mainGS()));
		SetPixelShader(CompileShader(ps_4_0, mainPS()));

		SetRasterizerState(RasterizerSettings);
		SetDepthStencilState(EnableDepth, 0);
	}
}