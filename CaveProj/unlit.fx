cbuffer ViewMatrix
{
	float4x4 View : View;
}

cbuffer WorldMatrix
{
	float4x4 World: World;
}
cbuffer ProjectionMatrix
{
	float4x4 Proj: Projection;
}

cbuffer Color
{
	float4 Color;
}

struct VS_INPUT
{
    float3 Pos          : POSITION;
};

struct PS_INPUT
{
	float4 Pos : SV_Position;
};

PS_INPUT mainVS(VS_INPUT input)
{
	float4x4 WorldViewProj = mul(World, mul(View, Proj));
    PS_INPUT output;
	output.Pos = mul(float4(input.Pos, 1), WorldViewProj);
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
	return Color;
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