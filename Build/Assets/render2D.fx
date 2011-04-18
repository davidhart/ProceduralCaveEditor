Texture2D tex;

sampler TextureSampler
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct VS_INPUT
{
    float2 Pos          : POSITION;
	float2 TexCoord		: TEXCOORD0;
	float4 Color		: COLOR0;
};

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
	output.Pos = float4(input.Pos, 0.0f, 1);
	output.TexCoord = input.TexCoord;
	output.Color = input.Color;
    return output;
}

float4 PSTextured(PS_INPUT input) : SV_TARGET
{
	return input.Color * tex.Sample(TextureSampler, input.TexCoord);
}

float4 PS(PS_INPUT input) : SV_TARGET
{
	return input.Color;
}

RasterizerState RasterizerSettings
{
    CullMode = NONE;
	ScissorEnable = TRUE;
	DepthClipEnable = FALSE;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

technique10 RenderTextured
{
	pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, mainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PSTextured()));
		
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(RasterizerSettings); 
		SetBlendState(AdditiveBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
	}
}

technique10 Render
{
	pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, mainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
		
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(RasterizerSettings); 
		SetBlendState(AdditiveBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
	}
}
