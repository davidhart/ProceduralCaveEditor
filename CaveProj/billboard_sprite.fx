cbuffer Matrices
{
	float4x4 WorldViewProjection;
	float4x4 InvView;
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

Texture2D Tex;

sampler TextureSampler = sampler_state
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_INPUT
{
    float3 Pos          : POSITION;
	float4 Color		: COLOR0;
	float Size			: TEXCOORD0;
};

struct GS_INPUT
{
	float3 Pos : POSITION;
	float4 Color : COLOR0;
	float Size: TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord: TEXCOORD0;
	float4 Color : COLOR0;
};

GS_INPUT mainVS(VS_INPUT input)
{
	GS_INPUT output;
	output.Pos = input.Pos;
	output.Color = input.Color;
	output.Size = input.Size;
    return output;
}

[maxvertexcount(4)]
void mainGS(point GS_INPUT input[1],
        inout TriangleStream<PS_INPUT> SpriteStream)
{
	PS_INPUT output;
    [unroll] for(int i=0; i<4; i++)
    {
        float3 position = VertPos[i]*input[0].Size;
        position = mul(position, (float3x3)InvView) + input[0].Pos;
        output.Pos = mul(float4(position, 1.0), WorldViewProjection);
        
        output.Color = input[0].Color;
        output.TexCoord = VertTexCoord[i];
        SpriteStream.Append(output);
    }
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{	
	float4 color = input.Color * Tex.Sample(TextureSampler, input.TexCoord);

	if (color.a <= 0.0f)
		discard;

	return color;
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
		SetGeometryShader(CompileShader(gs_4_0, mainGS()));
		SetPixelShader(CompileShader(ps_4_0, mainPS()));

		SetRasterizerState(RasterizerSettings);
		SetDepthStencilState(EnableDepth, 0);
	}
}